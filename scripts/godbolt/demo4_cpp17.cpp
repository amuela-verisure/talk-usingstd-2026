// Demo 4 C++17: Runtime calibration table in .bss
// Godbolt: arm-none-eabi-gcc 13.2
// Flags: -std=c++17 -Os -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti
//
// LOOK FOR: init_calibration() function body in assembly (~30 instructions).
// The 256-entry cal_table lives in .bss (zero-initialized, filled at runtime).

#include <array>
#include <cstdint>
#include <cstring>

struct SteinhartCoeffs {
    float A;
    float B;
    float C;
};

constexpr int ADC_TABLE_SIZE = 256;
constexpr float CALIBRATION_FALLBACK_TEMP = 25.0f;

[[nodiscard]] inline float steinhart_hart(float R, const SteinhartCoeffs& c) noexcept {
    float lnR = __builtin_logf(R);
    float lnR3 = lnR * lnR * lnR;
    return 1.0f / (c.A + c.B * lnR + c.C * lnR3);
}

[[nodiscard]] inline float adc_to_resistance(int adc_code) noexcept {
    constexpr float R_REF = 10000.0f;
    float ratio = static_cast<float>(adc_code) / 255.0f;
    if (ratio <= 0.001f) ratio = 0.001f;
    if (ratio >= 0.999f) ratio = 0.999f;
    return R_REF * ratio / (1.0f - ratio);
}

// This array lives in .bss -- 1024 bytes of RAM, zero-initialized.
std::array<float, ADC_TABLE_SIZE> cal_table{};

// This function appears in .text -- must run at startup before first ADC read.
void init_calibration(const SteinhartCoeffs& coeffs) noexcept {
    for (int i = 0; i < ADC_TABLE_SIZE; ++i) {
        float T = steinhart_hart(adc_to_resistance(i), coeffs);
        uint32_t bits;
        std::memcpy(&bits, &T, sizeof(bits));
        if ((bits & 0x7F800000u) == 0x7F800000u) {
            T = CALIBRATION_FALLBACK_TEMP;
        }
        cal_table[static_cast<std::size_t>(i)] = T;
    }
}

[[nodiscard]] float lookup_temperature(uint16_t adc_value) noexcept {
    uint8_t index = static_cast<uint8_t>(adc_value >> 4);
    return cal_table[index];
}
