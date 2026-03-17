#pragma once
#include <array>

// 32 canned ADC readings: mix of in-range and out-of-range values.
// Valid range: [200.0, 3800.0]. After filtering, 24 remain (>= SLIDING_WINDOW_SIZE + 4).
constexpr std::array<float, 32> test_readings = {
    100.0f,   // OUT - below 200
    250.0f,   // IN
    500.0f,   // IN
    750.0f,   // IN
    1000.0f,  // IN
    1250.0f,  // IN
    50.0f,    // OUT - below 200
    1500.0f,  // IN
    1750.0f,  // IN
    2000.0f,  // IN
    2250.0f,  // IN
    2500.0f,  // IN
    4000.0f,  // OUT - above 3800
    2750.0f,  // IN
    3000.0f,  // IN
    3250.0f,  // IN
    3500.0f,  // IN
    3750.0f,  // IN
    3900.0f,  // OUT - above 3800
    200.0f,   // IN  - exact boundary
    400.0f,   // IN
    600.0f,   // IN
    800.0f,   // IN
    1100.0f,  // IN
    1300.0f,  // IN
    1600.0f,  // IN
    1800.0f,  // IN
    2100.0f,  // IN
    5000.0f,  // OUT - far above
    3800.0f,  // IN  - exact boundary
    3600.0f,  // IN
    3400.0f,  // IN
};
// 24 in-range values -> after sliding window of 8: 17 averaged outputs.
