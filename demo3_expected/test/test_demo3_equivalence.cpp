#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "demo3/cpp17/sensor_init.hpp"
#include "demo3/cpp20/sensor_init.hpp"
#include "mock_i2c_device.hpp"

static void check_equivalent(FaultScenario scenario, Mode expected_mode) {
    hw::I2CDevice dev17;
    hw::I2CDevice dev20;

    configure_device_for_scenario(dev17, scenario);
    configure_device_for_scenario(dev20, scenario);

    auto result17 = demo3::cpp17::init_sensor(dev17);
    auto result20 = demo3::cpp20::init_sensor(dev20);

    CHECK(result17.mode == expected_mode);
    CHECK(result20.mode == expected_mode);
    CHECK(result17 == result20);
}

TEST_CASE("AllOk: both versions return Sampling") {
    check_equivalent(FaultScenario::AllOk, Mode::Sampling);
}

TEST_CASE("ConfigureFails: both versions return Offline (writes blocked)") {
    // configure() fails because write_fail is set. safe_mode_config() also
    // fails because it needs to write registers. Both go to Offline.
    check_equivalent(FaultScenario::ConfigureFails, Mode::Offline);
}

TEST_CASE("ReadCalibrationFails: both versions return Degraded") {
    check_equivalent(FaultScenario::ReadCalibrationFails, Mode::Degraded);
}

TEST_CASE("ReadOffsetFails: both versions return Degraded") {
    check_equivalent(FaultScenario::ReadOffsetFails, Mode::Degraded);
}

TEST_CASE("ValidateFails: both versions return Degraded") {
    check_equivalent(FaultScenario::ValidateFails, Mode::Degraded);
}

TEST_CASE("SafeModeFails: both versions return Offline") {
    check_equivalent(FaultScenario::SafeModeFails, Mode::Offline);
}

TEST_CASE("Monadic surface: all four operators used in C++20 version") {
    // This test verifies the C++20 code compiles with and_then, transform,
    // or_else, and value_or. The init_sensor function uses all four.
    // A successful path exercises and_then + transform.
    // A failure path exercises or_else + value_or.
    hw::I2CDevice dev_ok;
    configure_device_for_scenario(dev_ok, FaultScenario::AllOk);
    auto ok_result = demo3::cpp20::init_sensor(dev_ok);
    CHECK(ok_result.mode == Mode::Sampling);

    hw::I2CDevice dev_fail;
    configure_device_for_scenario(dev_fail, FaultScenario::SafeModeFails);
    auto fail_result = demo3::cpp20::init_sensor(dev_fail);
    CHECK(fail_result.mode == Mode::Offline);
}
