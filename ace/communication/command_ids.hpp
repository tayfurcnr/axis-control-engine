#pragma once

#include <cstdint>
#include <cstring>

namespace ace::communication {

enum class CommandId : std::uint8_t {
    UNKNOWN = 0x00,
    ENABLE = 0x10,
    DISABLE = 0x11,
    STOP = 0x12,
    SET_ANGLE = 0x20,
    SET_VELOCITY = 0x21,
    HOME = 0x22,
    SET_TARGET = 0x23,
    SET_MODE = 0x24,
    CALIBRATE = 0x30,
    SET_PID = 0x31,
    SET_MOTION_LIMITS = 0x32,
    GET_TELEMETRY = 0x40,
    HEARTBEAT = 0x50,
    SET_LOCATION = 0x60,
    SET_DEBUG = 0x61,
    SET_SERIAL = 0x62,
    ERROR = 0x70,
    GET_INFO = 0x80,
    REBOOT = 0x90,
    FACTORY_RESET = 0x91,
};

enum class AxisId : std::uint8_t {
    ALL = 0x00,
    PAN = 0x01,
    TILT = 0x02,
};

enum class ModeId : std::uint8_t {
    MANUAL = 0x01,
    AUTO = 0x02,
    TRACK = 0x03,
};

enum class StopTypeId : std::uint8_t {
    SOFT = 0x01,
    HARD = 0x02,
    EMERGENCY = 0x03,
};

enum class EventTypeId : std::uint8_t {
    MOTION_DONE = 0x01,
    TARGET_REACHED = 0x02,
};

enum class ErrorGroupId : std::uint8_t {
    MOTION = 0x01,
    SENSOR = 0x02,
    COMMUNICATION = 0x03,
    POWER = 0x04,
    STORAGE = 0x05,
};

enum class ErrorModuleId : std::uint8_t {
    SYSTEM = 0x00,
    PAN = 0x01,
    TILT = 0x02,
    IMU = 0x03,
    ENCODER = 0x04,
    COMM = 0x05,
    GPS = 0x06,
    FLASH = 0x07,
    NVS = 0x08,
    WATCHDOG = 0x09,
    POWER_SUPPLY = 0x0A,
};

enum class ErrorReasonId : std::uint8_t {
    TIMEOUT = 0x01,
    LIMIT = 0x02,
    CRC = 0x03,
    INVALID_PARAMETER = 0x04,
    OVERCURRENT = 0x05,
    BUSY = 0x06,
    NOT_INITIALIZED = 0x07,
    READ_FAIL = 0x08,
    WRITE_FAIL = 0x09,
    VERSION_MISMATCH = 0x0A,
    UNSUPPORTED = 0x0B,
};

enum class ErrorSeverityId : std::uint8_t {
    INFO = 0x01,
    WARNING = 0x02,
    ERROR = 0x03,
    FATAL = 0x04,
};

}  // namespace ace::communication
