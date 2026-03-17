#pragma once
#include <cstdint>

enum class ErrorCode : uint8_t {
    Ok              = 0x00,
    I2CNack         = 0x01,
    I2CTimeout      = 0x02,
    InvalidConfig   = 0x03,
    CalibrationFail = 0x04,
    OffsetOutOfRange= 0x05,
    ValidationFail  = 0x06,
    HardwareFault   = 0x07,
    NaN             = 0x08,
};

[[nodiscard]] constexpr bool is_ok(ErrorCode e) {
    return e == ErrorCode::Ok;
}
