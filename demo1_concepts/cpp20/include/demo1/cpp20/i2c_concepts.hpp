#pragma once
#include <concepts>
#include <cstdint>

template<typename T>
concept I2CSensor = requires(T dev, const T cdev) {
    { cdev.read_register(uint8_t{}) } -> std::same_as<uint16_t>;
    { dev.write_register(uint8_t{}, uint16_t{}) } -> std::same_as<bool>;
    { T::device_address } -> std::convertible_to<uint8_t>;
};
