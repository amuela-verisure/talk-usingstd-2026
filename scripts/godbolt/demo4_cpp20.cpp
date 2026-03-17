// Demo 4 C++20: Compile-time calibration table in .rodata
// Godbolt: arm-none-eabi-gcc 13.2
// Flags: -std=c++20 -Os -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti
//
// LOOK FOR: gen_cal_table does NOT appear in assembly (consteval -- computed at compile time).
// flash_cal_table appears in .rodata (flash, not RAM). No init function needed.

#include <array>
#include <bit>
#include <cstdint>
#include <span>

struct SteinhartCoeffs {
    float A;
    float B;
    float C;
};

constexpr int ADC_TABLE_SIZE = 256;

[[nodiscard]] constexpr float steinhart_hart(float R, const SteinhartCoeffs& c) noexcept {
    float lnR = __builtin_logf(R);
    float lnR3 = lnR * lnR * lnR;
    return 1.0f / (c.A + c.B * lnR + c.C * lnR3);
}

[[nodiscard]] constexpr float adc_to_resistance(int adc_code) noexcept {
    constexpr float R_REF = 10000.0f;
    float ratio = static_cast<float>(adc_code) / 255.0f;
    if (ratio <= 0.001f) ratio = 0.001f;
    if (ratio >= 0.999f) ratio = 0.999f;
    return R_REF * ratio / (1.0f - ratio);
}

// consteval: entire table computed at compile time. NaN = compile error.
consteval auto gen_cal_table(SteinhartCoeffs c)
    -> std::array<float, ADC_TABLE_SIZE>
{
    std::array<float, ADC_TABLE_SIZE> t{};
    for (int i = 0; i < ADC_TABLE_SIZE; ++i) {
        float T = steinhart_hart(adc_to_resistance(i), c);
        auto b = std::bit_cast<uint32_t>(T);
        if ((b & 0x7F800000u) == 0x7F800000u) {
            // Force compile-time error in consteval context.
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            throw "NaN or Inf detected in calibration table";
#else
            extern void nan_or_inf_in_calibration_table();
            nan_or_inf_in_calibration_table();
#endif
        }
        t[static_cast<std::size_t>(i)] = T;
    }
    return t;
}

inline constexpr SteinhartCoeffs default_coeffs{
    .A = 1.009249e-3f,
    .B = 2.378405e-4f,
    .C = 2.019202e-7f,
};

// constexpr -> .rodata (flash). Zero RAM cost. No init function.
inline constexpr auto flash_cal_table = gen_cal_table(default_coeffs);

[[nodiscard]] std::span<const float> get_flash_table() noexcept {
    return flash_cal_table;
}

[[nodiscard]] float lookup_temperature(std::span<const float> table,
                                       uint16_t adc_value) noexcept {
    auto shifted = static_cast<uint16_t>(adc_value >> 4);
    if (shifted > 255) __builtin_abort();
    auto index = static_cast<uint8_t>(shifted);
    return table[index];
}
