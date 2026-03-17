#pragma once
#include "common/hw_registers.hpp"
#include <cstdint>

enum class FaultScenario : uint8_t {
    AllOk,                // All steps succeed -> Mode::Sampling
    ConfigureFails,       // configure() fails -> safe_mode -> Mode::Degraded
    ReadCalibrationFails, // read_calibration() fails -> safe_mode -> Mode::Degraded
    ReadOffsetFails,      // read_offset() fails -> safe_mode -> Mode::Degraded
    ValidateFails,        // validate() fails -> safe_mode -> Mode::Degraded
    SafeModeFails,        // primary + safe_mode both fail -> Mode::Offline
};

inline void configure_device_for_scenario(
    hw::I2CDevice& dev,
    FaultScenario scenario) noexcept
{
    // Reset device state.
    dev.set_write_fail(false);

    // Default: all status bits set.
    uint16_t status = hw::STATUS_CAL_VALID | hw::STATUS_OFFSET_VALID
                    | hw::STATUS_VALIDATED | hw::STATUS_SAFE_MODE;

    // Default config registers for a "good" device.
    dev.set_register(hw::REG_SAMPLE_RATE, 10);
    dev.set_register(hw::REG_RESOLUTION, 14);

    switch (scenario) {
    case FaultScenario::AllOk:
        break;

    case FaultScenario::ConfigureFails:
        dev.set_write_fail(true);
        status = hw::STATUS_SAFE_MODE; // safe_mode bit set, but writes will fail
        break;

    case FaultScenario::ReadCalibrationFails:
        status = hw::STATUS_OFFSET_VALID | hw::STATUS_VALIDATED | hw::STATUS_SAFE_MODE;
        break;

    case FaultScenario::ReadOffsetFails:
        status = hw::STATUS_CAL_VALID | hw::STATUS_VALIDATED | hw::STATUS_SAFE_MODE;
        break;

    case FaultScenario::ValidateFails:
        status = hw::STATUS_CAL_VALID | hw::STATUS_OFFSET_VALID | hw::STATUS_SAFE_MODE;
        break;

    case FaultScenario::SafeModeFails:
        status = 0x00; // all bits clear
        dev.set_write_fail(true);
        break;
    }

    dev.set_register(hw::REG_STATUS, status);
}
