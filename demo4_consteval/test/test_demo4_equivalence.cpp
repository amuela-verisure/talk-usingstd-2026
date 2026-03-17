#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "demo4/cpp17/calibration_table.hpp"
#include "demo4/cpp20/calibration_table.hpp"
#include "common/platform.hpp"
#include "common/sensor_types.hpp"
#include <cmath>
#include <cstddef>
#include <span>

constexpr float EPSILON = 0.01f; // Temperature tolerance in Kelvin

// Steinhart-Hart coefficients matching the C++20 defaults.
static const SteinhartCoeffs test_coeffs{
    .A = 1.009249e-3f,
    .B = 2.378405e-4f,
    .C = 2.019202e-7f,
};

TEST_CASE("C++17 and C++20 calibration tables produce matching values") {
    // Initialize the C++17 runtime table.
    demo4::cpp17::init_calibration(test_coeffs);

    // Get the C++20 flash table (computed at compile time).
    auto flash_table = demo4::cpp20::get_flash_table();

    REQUIRE(flash_table.size() == ADC_TABLE_SIZE);

    for (int i = 0; i < ADC_TABLE_SIZE; ++i) {
        float cpp17_val = demo4::cpp17::cal_table[static_cast<std::size_t>(i)];
        float cpp20_val = flash_table[static_cast<std::size_t>(i)];
        CHECK(std::fabs(cpp17_val - cpp20_val) < EPSILON);
    }
}

TEST_CASE("C++20 constinit runtime table matches flash table") {
    auto flash_table   = demo4::cpp20::get_flash_table();
    auto runtime_table = demo4::cpp20::get_runtime_table();

    REQUIRE(flash_table.size() == runtime_table.size());

    for (std::size_t i = 0; i < flash_table.size(); ++i) {
        CHECK(flash_table[i] == runtime_table[i]);
    }
}

TEST_CASE("C++20 flash table is computed at compile time") {
    // This is verified by the constexpr/consteval nature of gen_cal_table.
    // If it weren't constexpr, flash_cal_table wouldn't compile.
    static_assert(demo4::cpp20::flash_cal_table.size() == ADC_TABLE_SIZE);
    static_assert(demo4::cpp20::flash_cal_table[0] > 0.0f);
    static_assert(demo4::cpp20::flash_cal_table[128] > 0.0f);
}

TEST_CASE("C++20 runtime table is mutable (field recalibration)") {
    float original = demo4::cpp20::runtime_cal_table[0];
    demo4::cpp20::runtime_cal_table[0] = 999.0f;
    CHECK(demo4::cpp20::runtime_cal_table[0] == 999.0f);
    demo4::cpp20::runtime_cal_table[0] = original; // restore
}

TEST_CASE("C++20 lookup_temperature with span works correctly") {
    auto table = demo4::cpp20::get_flash_table();

    // ADC value 0x00 >> 4 = 0 -> index 0
    float t0 = demo4::cpp20::lookup_temperature(table, 0x00);
    CHECK(t0 == table[0]);

    // ADC value 0x80 >> 4 = 8 -> index 8
    float t8 = demo4::cpp20::lookup_temperature(table, 0x80);
    CHECK(t8 == table[8]);

    // ADC value 0xFF0 >> 4 = 0xFF = 255 -> index 255
    float t255 = demo4::cpp20::lookup_temperature(table, 0xFF0);
    CHECK(t255 == table[255]);
}

TEST_CASE("C++17 lookup uses unchecked cast (demo contrast)") {
    demo4::cpp17::init_calibration(test_coeffs);
    // Valid index.
    float t = demo4::cpp17::lookup_temperature(0x80);
    CHECK(t == demo4::cpp17::cal_table[8]); // 0x80 >> 4 = 8
}

TEST_CASE("All table values are finite (no NaN/Inf)") {
    demo4::cpp17::init_calibration(test_coeffs);

    for (int i = 0; i < ADC_TABLE_SIZE; ++i) {
        CHECK(std::isfinite(demo4::cpp17::cal_table[static_cast<std::size_t>(i)]));
    }

    auto flash = demo4::cpp20::get_flash_table();
    for (std::size_t i = 0; i < flash.size(); ++i) {
        CHECK(std::isfinite(flash[i]));
    }
}
