// Demo 1 C++20: Concept constraint error diagnostic
// Godbolt: arm-none-eabi-gcc 13.2
// Flags: -std=c++20 -Os -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti
//
// THIS CODE DELIBERATELY FAILS TO COMPILE.
// LOOK FOR: Clear diagnostic telling you EXACTLY which constraints are unsatisfied:
//   - "read_register does not satisfy return-type-requirement" (returns void, not uint16_t)
//   - "the required expression 'T::device_address' is invalid" (missing member)
// Compare with the C++17 version's unhelpful "no type named 'type' in enable_if".

#include <concepts>
#include <cstdint>

// --- Concept (10 lines to express 3 constraints) ---

template<typename T>
concept I2CSensor = requires(T dev, const T cdev) {
    { cdev.read_register(uint8_t{}) } -> std::same_as<uint16_t>;
    { dev.write_register(uint8_t{}, uint16_t{}) } -> std::same_as<bool>;
    { T::device_address } -> std::convertible_to<uint8_t>;
};

// --- Driver with concept constraint ---

template<I2CSensor T>
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

// --- This instantiation fails with a clear concept error ---
BadSensor bad;
I2CDriver<BadSensor> driver(bad);
