#pragma once
#include "common/sensor_types.hpp"
#include "common/error_code.hpp"
#include <cstdint>

namespace demo1::cpp17 {

[[nodiscard]] inline bool validate_sample_rate(uint8_t rate) {
    return rate == 1 || rate == 10 || rate == 50 || rate == 100;
}

[[nodiscard]] inline bool validate_resolution(uint8_t bits) {
    return bits == 12 || bits == 14 || bits == 16;
}

[[nodiscard]] inline ErrorCode validate_config(const SensorConfig& cfg) {
    if (!validate_sample_rate(cfg.sample_rate)) return ErrorCode::InvalidConfig;
    if (!validate_resolution(cfg.resolution_bits)) return ErrorCode::InvalidConfig;
    return ErrorCode::Ok;
}

} // namespace demo1::cpp17
