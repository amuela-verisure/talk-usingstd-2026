#pragma once
#include "common/platform.hpp"
#include <array>
#include <cstddef>

namespace demo2::cpp17 {

[[nodiscard]] inline bool in_range(float sample) {
    return sample >= HUMIDITY_MIN_RAW && sample <= HUMIDITY_MAX_RAW;
}

[[nodiscard]] inline float calibrate(float raw) {
    constexpr float gain   = 0.02441f;   // ~100.0 / 4096.0
    constexpr float offset = -10.0f;
    return gain * raw + offset;
}

// Intermediate-buffer approach: three loops, two scratch arrays.
// Returns the number of valid averaged samples written to output.
std::size_t process_readings(
    const float* raw,
    std::size_t  raw_count,
    float*       output,
    std::size_t  output_capacity);

} // namespace demo2::cpp17
