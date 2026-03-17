#pragma once
#include "demo1/cpp20/i2c_concepts.hpp"
#include "common/error_code.hpp"
#include "common/sensor_types.hpp"
#include "common/hw_registers.hpp"
#include <cstdint>

template<I2CSensor T>
class I2CDriverCpp20 {
public:
    explicit I2CDriverCpp20(T& device) : device_(device) {}

    [[nodiscard]] uint16_t read(uint8_t reg) const {
        return device_.read_register(reg);
    }

    [[nodiscard]] ErrorCode write(uint8_t reg, uint16_t value) {
        return device_.write_register(reg, value)
            ? ErrorCode::Ok
            : ErrorCode::I2CNack;
    }

    [[nodiscard]] CalibrationData read_calibration() const {
        CalibrationData cal{};
        uint16_t ol = device_.read_register(hw::REG_CAL_OFFSET_L);
        uint16_t oh = device_.read_register(hw::REG_CAL_OFFSET_H);
        uint16_t gl = device_.read_register(hw::REG_CAL_GAIN_L);
        uint16_t gh = device_.read_register(hw::REG_CAL_GAIN_H);

        uint32_t offset_bits = (static_cast<uint32_t>(oh) << 16) | ol;
        uint32_t gain_bits   = (static_cast<uint32_t>(gh) << 16) | gl;

        static_assert(sizeof(float) == sizeof(uint32_t));
        __builtin_memcpy(&cal.offset, &offset_bits, sizeof(float));
        __builtin_memcpy(&cal.gain, &gain_bits, sizeof(float));

        cal.checksum = device_.read_register(hw::REG_CAL_CHECKSUM);
        return cal;
    }

    [[nodiscard]] static constexpr uint8_t address() {
        return T::device_address;
    }

private:
    T& device_;
};
