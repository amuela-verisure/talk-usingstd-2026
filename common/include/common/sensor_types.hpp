#pragma once
#include <cstdint>

struct SteinhartCoeffs {
    float A;
    float B;
    float C;
};

struct SensorReading {
    uint16_t raw;
    uint32_t timestamp_us;
};

struct CalibrationData {
    float offset;
    float gain;
    uint16_t checksum;
};

enum class Mode : uint8_t {
    Offline  = 0,
    Degraded = 1,
    Sampling = 2,
};

struct SensorConfig {
    uint8_t  sample_rate;
    uint8_t  resolution_bits;
    uint16_t integration_time_us;
};

struct SensorState {
    SensorConfig config;
    Mode         mode;

    static constexpr SensorState offline() {
        return SensorState{{0, 0, 0}, Mode::Offline};
    }
};

[[nodiscard]] constexpr bool operator==(const SensorConfig& a, const SensorConfig& b) {
    return a.sample_rate == b.sample_rate &&
           a.resolution_bits == b.resolution_bits &&
           a.integration_time_us == b.integration_time_us;
}

[[nodiscard]] constexpr bool operator==(const SensorState& a, const SensorState& b) {
    return a.config == b.config && a.mode == b.mode;
}
