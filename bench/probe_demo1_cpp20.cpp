// Compilation-time probe: Demo 1 C++20 (concepts + driver).
// Forces template instantiation so the header-only library's cost is measured.
#include "demo1/cpp20/i2c_driver.hpp"
#include "demo1/cpp20/config_validation.hpp"

// Explicit instantiation forces the compiler to process the full template.
template class I2CDriverCpp20<hw::I2CDevice>;

static_assert(demo1::cpp20::validate_config(demo1::cpp20::default_cfg), "");
