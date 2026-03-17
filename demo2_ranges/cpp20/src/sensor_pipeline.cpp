#include "demo2/cpp20/sensor_pipeline.hpp"
#include <array>

namespace demo2::cpp20 {

std::size_t process_readings(
    std::span<const float> raw,
    float*                 output,
    std::size_t            output_capacity)
{
    if (raw.empty() || output_capacity == 0) return 0;

    namespace views = ranges::views;

    // Materialize filtered+calibrated values first.
    // range-v3's views::sliding requires a forward range with known size;
    // filter is an input range whose length is unknown until consumed.
    // Materializing keeps the pipeline pattern while avoiding the
    // views::sliding assertion on empty/short ranges.
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

} // namespace demo2::cpp20
