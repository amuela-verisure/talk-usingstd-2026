#pragma once
#include "common/error_code.hpp"
#include "common/sensor_types.hpp"
#include "common/hw_registers.hpp"

namespace demo3::cpp17 {

[[nodiscard]] ErrorCode configure(hw::I2CDevice& dev);
[[nodiscard]] ErrorCode read_calibration(hw::I2CDevice& dev);
[[nodiscard]] ErrorCode read_offset(hw::I2CDevice& dev);
[[nodiscard]] ErrorCode validate(hw::I2CDevice& dev);

[[nodiscard]] SensorConfig get_config(hw::I2CDevice& dev);

void log_init_failure(hw::I2CDevice& dev, ErrorCode err);

[[nodiscard]] ErrorCode safe_mode_config(hw::I2CDevice& dev);

// Top-level init: nested-if "arrow anti-pattern."
[[nodiscard]] SensorState init_sensor(hw::I2CDevice& dev);

} // namespace demo3::cpp17
