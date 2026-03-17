// Demo 1 C++17: SFINAE constraint error diagnostic
// Godbolt: arm-none-eabi-gcc 13.2
// Flags: -std=c++17 -Os -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti
//
// THIS CODE DELIBERATELY FAILS TO COMPILE.
// LOOK FOR: Cryptic "enable_if" / "no type named 'type'" error.
// The diagnostic does NOT tell you WHICH constraint failed or WHY.
// Compare with the C++20 concept version's clear, actionable diagnostic.

#include <type_traits>
#include <cstdint>

// --- SFINAE traits (46 lines to express 3 constraints) ---

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

template<typename T, typename = void>
struct has_device_address : std::false_type {};

template<typename T>
struct has_device_address<T, std::void_t<
    std::enable_if_t<std::is_convertible_v<decltype(T::device_address), uint8_t>>
>> : std::true_type {};

template<typename T>
inline constexpr bool has_device_address_v = has_device_address<T>::value;

// --- Driver with SFINAE constraint ---

template<typename T, typename = std::enable_if_t<
    has_read_register_v<T> &&
    has_write_register_v<T> &&
    has_device_address_v<T>>>
class I2CDriver {
public:
    explicit I2CDriver(T& device) noexcept : device_(device) {}
    [[nodiscard]] uint16_t read(uint8_t reg) const noexcept {
        return device_.read_register(reg);
    }
private:
    T& device_;
};

// --- BadSensor: wrong return type, missing device_address ---

struct BadSensor {
    void read_register(uint8_t) const noexcept {}  // Wrong: returns void, not uint16_t
    bool write_register(uint8_t, uint16_t) noexcept { return false; }
    // Missing: static constexpr uint8_t device_address
};

// --- This instantiation fails with a wall of SFINAE errors ---
BadSensor bad;
I2CDriver<BadSensor> driver(bad);
