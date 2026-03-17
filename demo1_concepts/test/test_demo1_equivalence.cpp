#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "demo1/cpp17/i2c_traits.hpp"
#include "demo1/cpp17/i2c_driver.hpp"
#include "demo1/cpp17/config_validation.hpp"
#include "demo1/cpp20/i2c_concepts.hpp"
#include "demo1/cpp20/i2c_driver.hpp"
#include "demo1/cpp20/config_validation.hpp"
#include "common/error_code.hpp"
#include "common/hw_registers.hpp"
#include "common/sensor_types.hpp"
#include "mock_sensor.hpp"

// ----- Trait / Concept satisfaction -----

TEST_CASE("MockSensor satisfies C++17 traits") {
    CHECK(has_read_register_v<MockSensor>);
    CHECK(has_write_register_v<MockSensor>);
    CHECK(has_device_address_v<MockSensor>);
}

TEST_CASE("MockSensor satisfies C++20 I2CSensor concept") {
    CHECK(I2CSensor<MockSensor>);
}

TEST_CASE("BadSensor fails C++17 traits") {
    CHECK_FALSE(has_read_register_v<BadSensor>);
    CHECK_FALSE(has_device_address_v<BadSensor>);
}

TEST_CASE("BadSensor fails C++20 concept") {
    CHECK_FALSE(I2CSensor<BadSensor>);
}

TEST_CASE("hw::I2CDevice satisfies both trait and concept") {
    CHECK(has_read_register_v<hw::I2CDevice>);
    CHECK(has_write_register_v<hw::I2CDevice>);
    CHECK(has_device_address_v<hw::I2CDevice>);
    CHECK(I2CSensor<hw::I2CDevice>);
}

// ----- Functional equivalence -----

TEST_CASE("read() returns same value from both drivers") {
    MockSensor sensor;
    sensor.inject_register(0x10, 0xABCD);

    I2CDriverCpp17<MockSensor> drv17(sensor);
    I2CDriverCpp20<MockSensor> drv20(sensor);

    CHECK(drv17.read(0x10) == 0xABCD);
    CHECK(drv20.read(0x10) == 0xABCD);
    CHECK(drv17.read(0x10) == drv20.read(0x10));
}

TEST_CASE("write() returns same error code from both drivers") {
    MockSensor sensor;

    I2CDriverCpp17<MockSensor> drv17(sensor);
    I2CDriverCpp20<MockSensor> drv20(sensor);

    CHECK(drv17.write(0x02, 100) == ErrorCode::Ok);
    CHECK(drv20.write(0x02, 100) == ErrorCode::Ok);

    sensor.set_write_result(false);
    CHECK(drv17.write(0x02, 100) == ErrorCode::I2CNack);
    CHECK(drv20.write(0x02, 100) == ErrorCode::I2CNack);
}

TEST_CASE("read_calibration() returns identical data from both drivers") {
    MockSensor sensor;
    // Inject calibration register values.
    sensor.inject_register(hw::REG_CAL_OFFSET_L, 0x0000);
    sensor.inject_register(hw::REG_CAL_OFFSET_H, 0x3F80); // 1.0f upper half
    sensor.inject_register(hw::REG_CAL_GAIN_L,   0x0000);
    sensor.inject_register(hw::REG_CAL_GAIN_H,   0x4000); // 2.0f upper half
    sensor.inject_register(hw::REG_CAL_CHECKSUM,  0x1234);

    I2CDriverCpp17<MockSensor> drv17(sensor);
    I2CDriverCpp20<MockSensor> drv20(sensor);

    auto cal17 = drv17.read_calibration();
    auto cal20 = drv20.read_calibration();

    CHECK(cal17.offset == cal20.offset);
    CHECK(cal17.gain == cal20.gain);
    CHECK(cal17.checksum == cal20.checksum);
}

TEST_CASE("address() returns same value from both drivers") {
    CHECK(I2CDriverCpp17<MockSensor>::address() == 0x44);
    CHECK(I2CDriverCpp20<MockSensor>::address() == 0x44);
}

// ----- Config validation equivalence -----

TEST_CASE("Config validation agrees between C++17 and C++20") {
    // Valid configs.
    constexpr SensorConfig good{10, 14, 1000};
    CHECK(demo1::cpp17::validate_config(good) == ErrorCode::Ok);
    // C++20 validate_config is consteval, so test at compile time:
    static_assert(demo1::cpp20::validate_config(good));

    // Invalid sample rate.
    constexpr SensorConfig bad_rate{5, 14, 1000};
    CHECK(demo1::cpp17::validate_config(bad_rate) == ErrorCode::InvalidConfig);
    static_assert(!demo1::cpp20::validate_config(bad_rate));

    // Invalid resolution.
    constexpr SensorConfig bad_res{10, 8, 1000};
    CHECK(demo1::cpp17::validate_config(bad_res) == ErrorCode::InvalidConfig);
    static_assert(!demo1::cpp20::validate_config(bad_res));
}

TEST_CASE("SensorConfig is 4 bytes (hardware register width)") {
    static_assert(sizeof(SensorConfig) == 4);
}
