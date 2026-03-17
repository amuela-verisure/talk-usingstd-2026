#pragma once
#include <cstdint>

namespace hw {

class I2CDevice {
public:
    uint16_t read_register(uint8_t reg) const {
        return registers_[reg];
    }

    bool write_register(uint8_t reg, uint16_t value) {
        if (write_fail_) return false;
        registers_[reg] = value;
        return true;
    }

    static constexpr uint8_t device_address = 0x44;

    void set_register(uint8_t reg, uint16_t value) {
        registers_[reg] = value;
    }

    void set_write_fail(bool fail) {
        write_fail_ = fail;
    }

private:
    uint16_t registers_[256]{};
    bool write_fail_{false};
};

constexpr uint8_t REG_CONFIG        = 0x00;
constexpr uint8_t REG_STATUS        = 0x01;
constexpr uint8_t REG_SAMPLE_RATE   = 0x02;
constexpr uint8_t REG_RESOLUTION    = 0x03;
constexpr uint8_t REG_CAL_OFFSET_L  = 0x10;
constexpr uint8_t REG_CAL_OFFSET_H  = 0x11;
constexpr uint8_t REG_CAL_GAIN_L    = 0x12;
constexpr uint8_t REG_CAL_GAIN_H    = 0x13;
constexpr uint8_t REG_CAL_CHECKSUM  = 0x14;
constexpr uint8_t REG_MEASUREMENT_L = 0x20;
constexpr uint8_t REG_MEASUREMENT_H = 0x21;

// Status register bit masks.
constexpr uint16_t STATUS_CAL_VALID   = 0x01;
constexpr uint16_t STATUS_OFFSET_VALID = 0x02;
constexpr uint16_t STATUS_VALIDATED    = 0x04;
constexpr uint16_t STATUS_SAFE_MODE    = 0x08;

} // namespace hw
