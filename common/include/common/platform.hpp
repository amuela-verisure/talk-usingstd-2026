#pragma once
#include <cstddef>
#include <cstdint>

static_assert(sizeof(float) == 4, "This project requires 32-bit IEEE 754 floats");
static_assert(sizeof(uint32_t) == 4);

constexpr float CALIBRATION_FALLBACK_TEMP = 25.0f;

constexpr int ADC_RESOLUTION_BITS = 8;
constexpr int ADC_TABLE_SIZE = 1 << ADC_RESOLUTION_BITS; // 256

constexpr std::size_t MAX_READINGS = 64;
constexpr std::size_t SLIDING_WINDOW_SIZE = 8;

constexpr float HUMIDITY_MIN_RAW = 200.0f;
constexpr float HUMIDITY_MAX_RAW = 3800.0f;
