#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "demo2/cpp17/sensor_pipeline.hpp"
#include "demo2/cpp20/sensor_pipeline.hpp"
#include "test_data.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <span>

constexpr float EPSILON = 1e-5f;

TEST_CASE("Both versions produce identical output for test_readings") {
    std::array<float, 64> out17{};
    std::array<float, 64> out20{};

    auto n17 = demo2::cpp17::process_readings(
        test_readings.data(), test_readings.size(),
        out17.data(), out17.size());

    auto n20 = demo2::cpp20::process_readings(
        std::span<const float>(test_readings),
        out20.data(), out20.size());

    REQUIRE(n17 == n20);
    REQUIRE(n17 > 0);

    for (std::size_t i = 0; i < n17; ++i) {
        CHECK(std::fabs(out17[i] - out20[i]) < EPSILON);
    }
}

TEST_CASE("Empty input produces zero outputs") {
    std::array<float, 8> out17{};
    std::array<float, 8> out20{};

    auto n17 = demo2::cpp17::process_readings(
        nullptr, 0, out17.data(), out17.size());

    float empty_arr = 0.0f;
    auto n20 = demo2::cpp20::process_readings(
        std::span<const float>(), out20.data(), out20.size());

    CHECK(n17 == 0);
    CHECK(n20 == 0);
}

TEST_CASE("All out-of-range input produces zero outputs") {
    constexpr std::array<float, 4> bad = {100.0f, 50.0f, 4000.0f, 5000.0f};
    std::array<float, 8> out17{};
    std::array<float, 8> out20{};

    auto n17 = demo2::cpp17::process_readings(
        bad.data(), bad.size(), out17.data(), out17.size());

    auto n20 = demo2::cpp20::process_readings(
        std::span<const float>(bad), out20.data(), out20.size());

    CHECK(n17 == 0);
    CHECK(n20 == 0);
}

TEST_CASE("Exactly SLIDING_WINDOW_SIZE in-range inputs produce 1 output") {
    constexpr std::array<float, 8> exact = {
        300.0f, 400.0f, 500.0f, 600.0f,
        700.0f, 800.0f, 900.0f, 1000.0f
    };
    std::array<float, 8> out17{};
    std::array<float, 8> out20{};

    auto n17 = demo2::cpp17::process_readings(
        exact.data(), exact.size(), out17.data(), out17.size());

    auto n20 = demo2::cpp20::process_readings(
        std::span<const float>(exact), out20.data(), out20.size());

    CHECK(n17 == 1);
    CHECK(n20 == 1);
    CHECK(std::fabs(out17[0] - out20[0]) < EPSILON);
}

TEST_CASE("Fewer than SLIDING_WINDOW_SIZE in-range inputs produce 0 outputs") {
    constexpr std::array<float, 3> few = {300.0f, 400.0f, 500.0f};
    std::array<float, 8> out17{};
    std::array<float, 8> out20{};

    auto n17 = demo2::cpp17::process_readings(
        few.data(), few.size(), out17.data(), out17.size());

    auto n20 = demo2::cpp20::process_readings(
        std::span<const float>(few), out20.data(), out20.size());

    CHECK(n17 == 0);
    CHECK(n20 == 0);
}
