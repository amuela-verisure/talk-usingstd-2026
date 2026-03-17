// C++20 firmware entry point.
// Instantiates all C++20 demo code to produce a linkable .elf for size comparison.

#include "common/hw_registers.hpp"
#include "common/sensor_types.hpp"

#include "demo1/cpp20/i2c_driver.hpp"
#include "demo1/cpp20/config_validation.hpp"
#include "demo2/cpp20/sensor_pipeline.hpp"
#include "demo3/cpp20/sensor_init.hpp"
#include "demo4/cpp20/calibration_table.hpp"

#include <cstdint>
#include <span>

// Prevent the optimizer from removing unused results.
static volatile float sink;
static volatile uint16_t sink16;

int main() {
    // --- Demo 1: I2C driver with concepts ---
    hw::I2CDevice sensor;
    I2CDriverCpp20<hw::I2CDevice> driver(sensor);
    sink16 = driver.read(hw::REG_STATUS);
    auto wresult = driver.write(hw::REG_CONFIG, 0x01);
    static_cast<void>(wresult);
    auto cal = driver.read_calibration();
    sink = cal.offset;

    // Compile-time config validation (already checked via static_assert in header).

    // --- Demo 2: Ranges-v3 lazy pipeline ---
    float readings[16] = {
        250.0f, 500.0f, 750.0f, 1000.0f,
        1250.0f, 1500.0f, 1750.0f, 2000.0f,
        2250.0f, 2500.0f, 2750.0f, 3000.0f,
        3250.0f, 3500.0f, 3750.0f, 200.0f,
    };
    float output[16]{};
    auto nout = demo2::cpp20::process_readings(
        std::span<const float>(readings, 16), output, 16);
    if (nout > 0) sink = output[0];

    // --- Demo 3: Monadic chain sensor init ---
    auto state = demo3::cpp20::init_sensor(sensor);
    static_cast<void>(state);

    // --- Demo 4: Compile-time calibration table ---
    auto table = demo4::cpp20::get_flash_table();
    sink = demo4::cpp20::lookup_temperature(table, 0x80);

    while (true) {}
}
