#pragma once
#include "common/sensor_types.hpp"
#include <cstdint>

namespace demo1::cpp20 {

[[nodiscard]] consteval bool validate_sample_rate(uint8_t rate) {
    return rate == 1 || rate == 10 || rate == 50 || rate == 100;
}

[[nodiscard]] consteval bool validate_resolution(uint8_t bits) {
    return bits == 12 || bits == 14 || bits == 16;
}

[[nodiscard]] consteval bool validate_config(SensorConfig cfg) {
    return validate_sample_rate(cfg.sample_rate) &&
           validate_resolution(cfg.resolution_bits);
}

// SensorConfig must be exactly 4 bytes to match a 32-bit register read.
static_assert(sizeof(SensorConfig) == 4,
    "SensorConfig must be 4 bytes to match hardware register width");

// Compile-time validated default configuration.
constexpr SensorConfig default_cfg{
    .sample_rate = 10,
    .resolution_bits = 14,
    .integration_time_us = 1000
};
static_assert(validate_config(default_cfg), "Invalid default sensor configuration");

} // namespace demo1::cpp20
