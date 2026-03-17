#pragma once
#include "common/sensor_types.hpp"
#include "common/platform.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cmath>
#include <span>

namespace demo4::cpp20 {

[[nodiscard]] constexpr float steinhart_hart(float R, const SteinhartCoeffs& c) {
    // In constexpr context, std::log is not available pre-C++26.
    // GCC supports __builtin_logf in constexpr; Clang may not.
    // For host testing we use std::log; for consteval the compiler evaluates it.
    float lnR = __builtin_logf(R);
    float lnR3 = lnR * lnR * lnR;
    return 1.0f / (c.A + c.B * lnR + c.C * lnR3);
}

[[nodiscard]] constexpr float adc_to_resistance(int adc_code) {
    constexpr float R_REF = 10000.0f;
    float ratio = static_cast<float>(adc_code) / 255.0f;
    if (ratio <= 0.001f) ratio = 0.001f;
    if (ratio >= 0.999f) ratio = 0.999f;
    return R_REF * ratio / (1.0f - ratio);
}

// consteval: computed entirely at compile time. NaN detected via bit_cast.
// If a NaN is encountered, the throw makes the consteval fail at compile time.
consteval auto gen_cal_table(SteinhartCoeffs c)
    -> std::array<float, ADC_TABLE_SIZE>
{
    std::array<float, ADC_TABLE_SIZE> t{};
    for (int i = 0; i < ADC_TABLE_SIZE; ++i) {
        float T = steinhart_hart(adc_to_resistance(i), c);
        auto b = std::bit_cast<uint32_t>(T);
        if ((b & 0x7F800000u) == 0x7F800000u) {
            // Force compile-time error in consteval context.
            // throw requires -fexceptions; calling a non-constexpr function
            // in a consteval function is always a compile error.
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

// Default Steinhart-Hart coefficients for a typical 10k NTC thermistor.
inline constexpr SteinhartCoeffs default_coeffs{
    .A = 1.009249e-3f,
    .B = 2.378405e-4f,
    .C = 2.019202e-7f,
};

// constexpr table -> .rodata (flash). Immutable. Zero RAM cost.
inline constexpr auto flash_cal_table = gen_cal_table(default_coeffs);

// constinit table -> .data (initialized RAM). Mutable for field recalibration.
// Defined in calibration_table.cpp.
extern std::array<float, ADC_TABLE_SIZE> runtime_cal_table;

// Consumer: std::span decouples from storage.
[[nodiscard]] inline std::span<const float> get_flash_table() {
    return flash_cal_table;
}

[[nodiscard]] inline std::span<const float> get_runtime_table() {
    return runtime_cal_table;
}

// Lookup with gsl::narrow for checked index truncation.
[[nodiscard]] float lookup_temperature(std::span<const float> table,
                                       uint16_t adc_value);

} // namespace demo4::cpp20
