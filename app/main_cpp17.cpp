// C++17 firmware entry point.
// Instantiates all C++17 demo code to produce a linkable .elf for size comparison.

#include "common/hw_registers.hpp"
#include "common/sensor_types.hpp"

#include "demo1/cpp17/i2c_driver.hpp"
#include "demo1/cpp17/config_validation.hpp"
#include "demo2/cpp17/sensor_pipeline.hpp"
#include "demo3/cpp17/sensor_init.hpp"
#include "demo4/cpp17/calibration_table.hpp"

#include <cstdint>

// Prevent the optimizer from removing unused results.
static volatile float sink;
static volatile uint16_t sink16;

int main() {
    // --- Demo 1: I2C driver with SFINAE constraints ---
    hw::I2CDevice sensor;
    I2CDriverCpp17<hw::I2CDevice> driver(sensor);
    sink16 = driver.read(hw::REG_STATUS);
    auto wresult = driver.write(hw::REG_CONFIG, 0x01);
    static_cast<void>(wresult);
    auto cal = driver.read_calibration();
    sink = cal.offset;

    SensorConfig cfg{10, 14, 1000};
    auto verr = demo1::cpp17::validate_config(cfg);
    static_cast<void>(verr);

    // --- Demo 2: Loop-based sensor pipeline ---
    float readings[16] = {
        250.0f, 500.0f, 750.0f, 1000.0f,
        1250.0f, 1500.0f, 1750.0f, 2000.0f,
        2250.0f, 2500.0f, 2750.0f, 3000.0f,
        3250.0f, 3500.0f, 3750.0f, 200.0f,
    };
    float output[16]{};
    auto nout = demo2::cpp17::process_readings(readings, 16, output, 16);
    if (nout > 0) sink = output[0];

    // --- Demo 3: Nested-if sensor init ---
    auto state = demo3::cpp17::init_sensor(sensor);
    static_cast<void>(state);

    // --- Demo 4: Runtime calibration table ---
    SteinhartCoeffs coeffs{1.009249e-3f, 2.378405e-4f, 2.019202e-7f};
    demo4::cpp17::init_calibration(coeffs);
    sink = demo4::cpp17::lookup_temperature(0x80);

    while (true) {}
}
