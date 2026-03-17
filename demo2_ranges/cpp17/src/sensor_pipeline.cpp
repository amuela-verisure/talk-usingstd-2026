#include "demo2/cpp17/sensor_pipeline.hpp"

namespace demo2::cpp17 {

std::size_t process_readings(
    const float* raw,
    std::size_t  raw_count,
    float*       output,
    std::size_t  output_capacity)
{
    // Step 1: filter into intermediate buffer
    std::array<float, MAX_READINGS> filtered{};
    std::size_t fcount = 0;
    for (std::size_t i = 0; i < raw_count; ++i) {
        if (in_range(raw[i])) {
            if (fcount >= MAX_READINGS) break;
            filtered[fcount++] = raw[i];
        }
    }

    // Step 2: calibrate into second intermediate buffer
    std::array<float, MAX_READINGS> calibrated{};
    for (std::size_t i = 0; i < fcount; ++i) {
        calibrated[i] = calibrate(filtered[i]);
    }

    // Step 3: sliding window average
    std::size_t out_count = 0;
    if (fcount >= SLIDING_WINDOW_SIZE) {
        for (std::size_t i = 0;
             i <= fcount - SLIDING_WINDOW_SIZE && out_count < output_capacity;
             ++i)
        {
            float sum = 0.0f;
            for (std::size_t j = 0; j < SLIDING_WINDOW_SIZE; ++j) {
                sum += calibrated[i + j];
            }
            output[out_count++] = sum / static_cast<float>(SLIDING_WINDOW_SIZE);
        }
    }
    return out_count;
}

} // namespace demo2::cpp17
