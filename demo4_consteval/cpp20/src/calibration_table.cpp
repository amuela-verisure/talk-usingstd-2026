#include "demo4/cpp20/calibration_table.hpp"
#include <cstdlib>

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
#include <gsl/narrow>
#endif

namespace demo4::cpp20 {

// constinit: initialized at compile time, mutable at runtime.
// Lives in .data section (initialized RAM), not .bss.
constinit std::array<float, ADC_TABLE_SIZE> runtime_cal_table =
    gen_cal_table(default_coeffs);

float lookup_temperature(std::span<const float> table,
                         uint16_t adc_value)
{
    // Checked narrowing cast: truncate 12-bit ADC to 8-bit table index.
    // With exceptions: gsl::narrow throws on overflow.
    // Without exceptions: manual check + std::terminate.
    auto shifted = static_cast<uint16_t>(adc_value >> 4);
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
    auto index = gsl::narrow<uint8_t>(shifted);
#else
    if (shifted > UINT8_MAX) {
        std::abort();
    }
    auto index = static_cast<uint8_t>(shifted);
#endif
    return table[index];
}

} // namespace demo4::cpp20
