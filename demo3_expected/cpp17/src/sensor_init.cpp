#include "demo3/cpp17/sensor_init.hpp"
#include <cstdint>

namespace demo3::cpp17 {

// Each step reads/writes specific registers and checks for errors.
// Register layout convention:
//   REG_CONFIG bit 0 = configured flag
//   REG_STATUS bit 0 = calibration valid
//   REG_STATUS bit 1 = offset valid
//   REG_STATUS bit 2 = validation passed
//   REG_STATUS bit 3 = safe-mode available

ErrorCode configure(hw::I2CDevice& dev) {
    if (!dev.write_register(hw::REG_CONFIG, 0x01))
        return ErrorCode::I2CNack;
    return ErrorCode::Ok;
}

ErrorCode read_calibration(hw::I2CDevice& dev) {
    uint16_t status = dev.read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_CAL_VALID))
        return ErrorCode::CalibrationFail;
    return ErrorCode::Ok;
}

ErrorCode read_offset(hw::I2CDevice& dev) {
    uint16_t status = dev.read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_OFFSET_VALID))
        return ErrorCode::OffsetOutOfRange;
    return ErrorCode::Ok;
}

ErrorCode validate(hw::I2CDevice& dev) {
    uint16_t status = dev.read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_VALIDATED))
        return ErrorCode::ValidationFail;
    return ErrorCode::Ok;
}

SensorConfig get_config(hw::I2CDevice& dev) {
    SensorConfig cfg{};
    cfg.sample_rate = static_cast<uint8_t>(dev.read_register(hw::REG_SAMPLE_RATE));
    cfg.resolution_bits = static_cast<uint8_t>(dev.read_register(hw::REG_RESOLUTION));
    cfg.integration_time_us = 1000;
    return cfg;
}

void log_init_failure(hw::I2CDevice& /*dev*/, ErrorCode /*err*/) {
    // In production: write to a diagnostic register or ring buffer.
    // For this demo: no-op (side-effect observable in tests via mock).
}

ErrorCode safe_mode_config(hw::I2CDevice& dev) {
    uint16_t status = dev.read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_SAFE_MODE))
        return ErrorCode::HardwareFault;
    // Write a minimal safe-mode config.
    if (!dev.write_register(hw::REG_SAMPLE_RATE, 1))
        return ErrorCode::I2CNack;
    if (!dev.write_register(hw::REG_RESOLUTION, 12))
        return ErrorCode::I2CNack;
    return ErrorCode::Ok;
}

SensorState init_sensor(hw::I2CDevice& dev) {
    auto err = configure(dev);
    if (is_ok(err)) {
        err = read_calibration(dev);
        if (is_ok(err)) {
            err = read_offset(dev);
            if (is_ok(err)) {
                err = validate(dev);
                if (is_ok(err)) {
                    return SensorState{get_config(dev), Mode::Sampling};
                }
            }
        }
    }
    log_init_failure(dev, err);
    auto fb = safe_mode_config(dev);
    if (is_ok(fb)) {
        return SensorState{get_config(dev), Mode::Degraded};
    }
    return SensorState::offline();
}

} // namespace demo3::cpp17
