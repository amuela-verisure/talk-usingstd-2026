// Demo 2 C++17: Three loops + two intermediate buffers
// Godbolt: arm-none-eabi-gcc 13.2
// Flags: -std=c++17 -Os -DNDEBUG -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti
//
// LOOK FOR: Three separate loop structures in the assembly.
// Two std::array<float, 64> buffers on the stack (~512 bytes).
// Inner loop instructions: vldr, vcmp.f32, vmla.f32, vstr.

#include <array>
#include <cstddef>

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

std::size_t process_readings(
    const float* raw,
    std::size_t  raw_count,
    float*       output,
    std::size_t  output_capacity) noexcept
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
