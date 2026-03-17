// Demo 2 C++20: Ranges-v3 filter | transform | sliding pipeline
// Godbolt: arm-none-eabi-gcc 13.2
// Flags: -std=c++20 -Os -DNDEBUG -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti
// Library: Add range-v3 from the library picker
//
// LOOK FOR: Equivalent inner loop instructions (vldr, vcmp.f32, vmla.f32, vstr).
// One buffer instead of two. Same math, different syntax.

#include <array>
#include <cstddef>
#include <span>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/sliding.hpp>

constexpr std::size_t MAX_READINGS = 64;
constexpr std::size_t SLIDING_WINDOW_SIZE = 8;
constexpr float HUMIDITY_MIN_RAW = 200.0f;
constexpr float HUMIDITY_MAX_RAW = 3800.0f;

[[nodiscard]] inline bool in_range(float sample) noexcept {
    return sample >= HUMIDITY_MIN_RAW && sample <= HUMIDITY_MAX_RAW;
}

[[nodiscard]] inline float calibrate(float raw) noexcept {
    constexpr float gain   = 0.02441f;
    constexpr float offset = -10.0f;
    return gain * raw + offset;
}

struct window_avg_fn {
    template<typename Rng>
    [[nodiscard]] float operator()(Rng&& window) const noexcept {
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

std::size_t process_readings(
    std::span<const float> raw,
    float*                 output,
    std::size_t            output_capacity) noexcept
{
    if (raw.empty() || output_capacity == 0) return 0;

    namespace views = ranges::views;

    // Materialize filtered+calibrated into one buffer.
    // views::sliding requires a forward range; filter produces an input range.
    std::array<float, MAX_READINGS> calibrated{};
    std::size_t cal_count = 0;
    for (float v : raw | views::filter(in_range) | views::transform(calibrate)) {
        if (cal_count >= MAX_READINGS) break;
        calibrated[cal_count++] = v;
    }

    if (cal_count < SLIDING_WINDOW_SIZE) return 0;

    auto windowed = std::span<const float>(calibrated.data(), cal_count)
        | views::sliding(static_cast<std::ptrdiff_t>(SLIDING_WINDOW_SIZE))
        | views::transform(window_avg);

    std::size_t count = 0;
    for (float val : windowed) {
        if (count >= output_capacity) break;
        output[count++] = val;
    }
    return count;
}
