#pragma once
#include <cstdint>

struct MockSensor {
    static constexpr uint8_t device_address = 0x44;

    uint16_t read_register(uint8_t reg) const noexcept {
        return regs_[reg];
    }

    bool write_register(uint8_t reg, uint16_t value) noexcept {
        if (!write_ok_) return false;
        regs_[reg] = value;
        return true;
    }

    void inject_register(uint8_t reg, uint16_t value) noexcept {
        regs_[reg] = value;
    }

    void set_write_result(bool result) noexcept {
        write_ok_ = result;
    }

private:
    uint16_t regs_[256]{};
    bool write_ok_{true};
};

// Deliberately invalid sensor for negative tests.
struct BadSensor {
    // Missing device_address.
    void read_register(uint8_t) const noexcept {} // Wrong return type (void).
    bool write_register(uint8_t, uint16_t) noexcept { return false; }
};
