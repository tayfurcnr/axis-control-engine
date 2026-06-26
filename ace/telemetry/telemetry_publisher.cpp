#include "ace/telemetry/telemetry_publisher.hpp"

#include <cstdio>
#include <cmath>

namespace ace::telemetry {

// SUMMARY: DONE: AxisManager'dan eksen durum, mod ve hız verisini alıp PROTOCOL_REFERENCE standart telemetri formatında string serileştirir.
void TelemetryPublisher::publish_status(const ace::axis::AxisManager& axis_manager) const
{
    const auto pan = axis_manager.pan_status();
    const auto tilt = axis_manager.tilt_status();
    const auto mode = axis_manager.mode();

    const char* mode_str = "MANUAL";
    if (mode == ace::communication::ModeId::AUTO) mode_str = "AUTO";
    else if (mode == ace::communication::ModeId::TRACK) mode_str = "TRACK";

    bool moving = std::abs(pan.current_velocity_dps) > 0.01f || std::abs(tilt.current_velocity_dps) > 0.01f;
    bool enabled = pan.enabled || tilt.enabled;

    std::printf("TELEMETRY ENABLED=%d MOVING=%d PAN_ANGLE=%.2f TILT_ANGLE=%.2f PAN_VELOCITY=%.2f TILT_VELOCITY=%.2f TEMPERATURE=%.1f VOLTAGE=%.1f MODE=%s ERROR_CODE=0x00000000\n",
                enabled ? 1 : 0,
                moving ? 1 : 0,
                pan.current_position_deg,
                tilt.current_position_deg,
                pan.current_velocity_dps,
                tilt.current_velocity_dps,
                42.0f, // mock temp
                24.0f, // mock voltage
                mode_str);
}

}  // namespace ace::telemetry
