#pragma once
#include <type_traits>
#include <cstdint>

// --- Trait: has read_register(uint8_t) -> uint16_t ---
template<typename T, typename = void>
struct has_read_register : std::false_type {};

template<typename T>
struct has_read_register<T, std::void_t<
    std::enable_if_t<std::is_same_v<
        decltype(std::declval<const T&>().read_register(uint8_t{})),
        uint16_t
    >>
>> : std::true_type {};

template<typename T>
inline constexpr bool has_read_register_v = has_read_register<T>::value;

// --- Trait: has write_register(uint8_t, uint16_t) -> bool ---
template<typename T, typename = void>
struct has_write_register : std::false_type {};

template<typename T>
struct has_write_register<T, std::void_t<
    std::enable_if_t<std::is_same_v<
        decltype(std::declval<T&>().write_register(uint8_t{}, uint16_t{})),
        bool
    >>
>> : std::true_type {};

template<typename T>
inline constexpr bool has_write_register_v = has_write_register<T>::value;

// --- Trait: has static constexpr device_address convertible to uint8_t ---
template<typename T, typename = void>
struct has_device_address : std::false_type {};

template<typename T>
struct has_device_address<T, std::void_t<
    std::enable_if_t<std::is_convertible_v<decltype(T::device_address), uint8_t>>
>> : std::true_type {};

template<typename T>
inline constexpr bool has_device_address_v = has_device_address<T>::value;
