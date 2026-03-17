#pragma once
#include "common/error_code.hpp"
#include "common/sensor_types.hpp"
#include "common/hw_registers.hpp"
#include <tl/expected.hpp>
#include <functional>

namespace demo3::cpp20 {

using DevRef = std::reference_wrapper<hw::I2CDevice>;
using StepResult = tl::expected<DevRef, ErrorCode>;

[[nodiscard]] StepResult configure(hw::I2CDevice& dev);
[[nodiscard]] StepResult read_calibration(DevRef dev);
[[nodiscard]] StepResult read_offset(DevRef dev);
[[nodiscard]] StepResult validate(DevRef dev);

[[nodiscard]] SensorConfig get_config(hw::I2CDevice& dev);

void log_init_failure(hw::I2CDevice& dev, ErrorCode err);

[[nodiscard]] tl::expected<DevRef, ErrorCode>
    safe_mode_config(hw::I2CDevice& dev);

// Top-level init: monadic chain using and_then, transform, or_else, value_or.
[[nodiscard]] SensorState init_sensor(hw::I2CDevice& dev);

} // namespace demo3::cpp20
