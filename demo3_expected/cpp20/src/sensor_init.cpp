#include "demo3/cpp20/sensor_init.hpp"
#include <cstdint>

namespace demo3::cpp20 {

StepResult configure(hw::I2CDevice& dev) {
    if (!dev.write_register(hw::REG_CONFIG, 0x01))
        return tl::unexpected(ErrorCode::I2CNack);
    return DevRef{dev};
}

StepResult read_calibration(DevRef dev) {
    uint16_t status = dev.get().read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_CAL_VALID))
        return tl::unexpected(ErrorCode::CalibrationFail);
    return dev;
}

StepResult read_offset(DevRef dev) {
    uint16_t status = dev.get().read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_OFFSET_VALID))
        return tl::unexpected(ErrorCode::OffsetOutOfRange);
    return dev;
}

StepResult validate(DevRef dev) {
    uint16_t status = dev.get().read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_VALIDATED))
        return tl::unexpected(ErrorCode::ValidationFail);
    return dev;
}

SensorConfig get_config(hw::I2CDevice& dev) {
    SensorConfig cfg{};
    cfg.sample_rate = static_cast<uint8_t>(dev.read_register(hw::REG_SAMPLE_RATE));
    cfg.resolution_bits = static_cast<uint8_t>(dev.read_register(hw::REG_RESOLUTION));
    cfg.integration_time_us = 1000;
    return cfg;
}

void log_init_failure(hw::I2CDevice& /*dev*/, ErrorCode /*err*/) {
    // No-op for demo. Same as C++17 version.
}

tl::expected<DevRef, ErrorCode>
safe_mode_config(hw::I2CDevice& dev) {
    uint16_t status = dev.read_register(hw::REG_STATUS);
    if (!(status & hw::STATUS_SAFE_MODE))
        return tl::unexpected(ErrorCode::HardwareFault);
    if (!dev.write_register(hw::REG_SAMPLE_RATE, 1))
        return tl::unexpected(ErrorCode::I2CNack);
    if (!dev.write_register(hw::REG_RESOLUTION, 12))
        return tl::unexpected(ErrorCode::I2CNack);
    return DevRef{dev};
}

SensorState init_sensor(hw::I2CDevice& dev) {
    return configure(dev)
        .and_then(read_calibration)
        .and_then(read_offset)
        .and_then(validate)
        .transform([](DevRef ref) {
            return SensorState{get_config(ref.get()), Mode::Sampling};
        })
        .or_else([&](ErrorCode err)
            -> tl::expected<SensorState, ErrorCode> {
            log_init_failure(dev, err);
            return safe_mode_config(dev)
                .transform([](DevRef ref) {
                    return SensorState{get_config(ref.get()), Mode::Degraded};
                });
        })
        .value_or(SensorState::offline());
}

} // namespace demo3::cpp20
