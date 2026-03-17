#pragma once
#include "common/platform.hpp"
#include <cstddef>
#include <span>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/sliding.hpp>

namespace demo2::cpp20 {

[[nodiscard]] inline bool in_range(float sample) {
    return sample >= HUMIDITY_MIN_RAW && sample <= HUMIDITY_MAX_RAW;
}

[[nodiscard]] inline float calibrate(float raw) {
    constexpr float gain   = 0.02441f;
    constexpr float offset = -10.0f;
    return gain * raw + offset;
}

// Compute arithmetic mean of a sliding window subrange.
struct window_avg_fn {
    template<typename Rng>
    [[nodiscard]] float operator()(Rng&& window) const {
        float sum = 0.0f;
        std::size_t count = 0;
        for (float v : window) {
            sum += v;
            ++count;
        }
        return (count > 0) ? sum / static_cast<float>(count) : 0.0f;
    }
};

inline constexpr window_avg_fn window_avg{};

// Lazy filter+calibrate pipeline, materialized once for sliding window.
// One buffer (vs two in C++17). Returns count of values written.
std::size_t process_readings(
    std::span<const float> raw,
    float*                 output,
    std::size_t            output_capacity);

} // namespace demo2::cpp20
