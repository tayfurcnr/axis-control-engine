#pragma once

#include <cstdint>
#include <optional>

#include "ace/communication/command_ids.hpp"

namespace ace::communication {

struct TelemetryResponse {
    bool enabled = false;
    bool moving = false;
    float pan_angle_deg = 0.0f;
    float tilt_angle_deg = 0.0f;
    float pan_velocity_dps = 0.0f;
    float tilt_velocity_dps = 0.0f;
    float temperature_c = 0.0f;
    float voltage_v = 0.0f;
    ModeId mode = ModeId::MANUAL;
    std::uint32_t error_code = 0;
};

struct InfoResponse {
    const char* device_name = nullptr;
    std::uint16_t device_id = 0;
    const char* fw_version = nullptr;
    const char* protocol_version = nullptr;
    const char* build_date = nullptr;
    const char* build_time = nullptr;
    const char* board = nullptr;
    const char* features = nullptr;
};

struct EventMessage {
    EventTypeId type = EventTypeId::MOTION_DONE;
    CommandId command_id = CommandId::UNKNOWN;
};

struct ErrorCode {
    ErrorGroupId group = ErrorGroupId::MOTION;
    ErrorModuleId module = ErrorModuleId::SYSTEM;
    ErrorReasonId reason = ErrorReasonId::TIMEOUT;
    ErrorSeverityId severity = ErrorSeverityId::INFO;

    [[nodiscard]] constexpr std::uint32_t pack() const noexcept
    {
        return (static_cast<std::uint32_t>(group) << 24)
             | (static_cast<std::uint32_t>(module) << 16)
             | (static_cast<std::uint32_t>(reason) << 8)
             | static_cast<std::uint32_t>(severity);
    }

    [[nodiscard]] static constexpr ErrorCode unpack(std::uint32_t value) noexcept
    {
        return {
            static_cast<ErrorGroupId>((value >> 24) & 0xFF),
            static_cast<ErrorModuleId>((value >> 16) & 0xFF),
            static_cast<ErrorReasonId>((value >> 8) & 0xFF),
            static_cast<ErrorSeverityId>(value & 0xFF),
        };
    }
};

[[nodiscard]] constexpr ErrorCode make_error(ErrorGroupId group,
                                             ErrorModuleId module,
                                             ErrorReasonId reason,
                                             ErrorSeverityId severity) noexcept
{
    return {group, module, reason, severity};
}

struct AckResponse {
    CommandId command_id = CommandId::UNKNOWN;
};

struct NackResponse {
    CommandId command_id = CommandId::UNKNOWN;
    ErrorCode error {};
};

struct CommandRequest {
    CommandId command_id = CommandId::UNKNOWN;
    AxisId axis = AxisId::ALL;
    StopTypeId stop_type = StopTypeId::SOFT;
    ModeId mode = ModeId::MANUAL;
    float pan_angle_deg = 0.0f;
    float tilt_angle_deg = 0.0f;
    float pan_velocity_dps = 0.0f;
    float tilt_velocity_dps = 0.0f;
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;
    float feed_forward = 0.0f;
    float max_integral = 0.0f;
    float max_output = 0.0f;
    float max_acceleration_dps2 = 0.0f;
    double longitude_deg = 0.0;
    double latitude_deg = 0.0;
    double altitude_m = 0.0;
    bool debug_enabled = false;
    char serial_number[32] = {};
};

struct CommandOutcome {
    std::optional<AckResponse> ack;
    std::optional<NackResponse> nack;
    std::optional<EventMessage> event;
};

}  // namespace ace::communication
