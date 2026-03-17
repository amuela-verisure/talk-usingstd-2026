// Compilation-time probe: Demo 1 C++17 (SFINAE traits + driver).
// Forces template instantiation so the header-only library's cost is measured.
#include "demo1/cpp17/i2c_driver.hpp"
#include "demo1/cpp17/config_validation.hpp"

// Explicit instantiation forces the compiler to process the full template.
template class I2CDriverCpp17<hw::I2CDevice>;

// C++17 validate_config is a runtime function — call it to force codegen.
volatile bool cfg_ok = (demo1::cpp17::validate_config(SensorConfig{10, 14, 1000}) == ErrorCode::Ok);
