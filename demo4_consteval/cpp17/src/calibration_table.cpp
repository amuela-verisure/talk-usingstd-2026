#include "demo4/cpp17/calibration_table.hpp"

namespace demo4::cpp17 {

std::array<float, ADC_TABLE_SIZE> cal_table{};

void init_calibration(const SteinhartCoeffs& coeffs) {
    for (int i = 0; i < ADC_TABLE_SIZE; ++i) {
        float T = steinhart_hart(adc_to_resistance(i), coeffs);
        // Runtime NaN check via memcpy bit inspection.
        uint32_t bits;
        std::memcpy(&bits, &T, sizeof(bits));
        if ((bits & 0x7F800000u) == 0x7F800000u) {
            T = CALIBRATION_FALLBACK_TEMP;
        }
        cal_table[static_cast<std::size_t>(i)] = T;
    }
}

float lookup_temperature(uint16_t adc_value) {
    // Truncation: unchecked cast to uint8_t.
    uint8_t index = static_cast<uint8_t>(adc_value >> 4);
    return cal_table[index];
}

} // namespace demo4::cpp17
