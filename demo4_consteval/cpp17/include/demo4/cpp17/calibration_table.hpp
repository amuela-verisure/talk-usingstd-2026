#pragma once
#include "common/sensor_types.hpp"
#include "common/platform.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace demo4::cpp17 {

[[nodiscard]] inline float steinhart_hart(float R, const SteinhartCoeffs& c) {
    float lnR = std::log(R);
    float lnR3 = lnR * lnR * lnR;
    return 1.0f / (c.A + c.B * lnR + c.C * lnR3);
}

[[nodiscard]] inline float adc_to_resistance(int adc_code) {
    constexpr float R_REF = 10000.0f;
    float ratio = static_cast<float>(adc_code) / 255.0f;
    if (ratio <= 0.001f) ratio = 0.001f;
    if (ratio >= 0.999f) ratio = 0.999f;
    return R_REF * ratio / (1.0f - ratio);
}

// Runtime calibration table stored in .bss (zero-initialized, filled at startup).
extern std::array<float, ADC_TABLE_SIZE> cal_table;

// Must be called at startup before first ADC read.
void init_calibration(const SteinhartCoeffs& coeffs);

// Lookup with manual bounds check (raw pointer access).
[[nodiscard]] float lookup_temperature(uint16_t adc_value);

} // namespace demo4::cpp17
