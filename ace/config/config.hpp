#pragma once

#include <cstdint>

#include "ace/communication/command_ids.hpp"

namespace ace::config {

struct RuntimeConfig {
    float max_velocity_deg_s = 0.0f;
    float max_acceleration_deg_s2 = 0.0f;
    bool debug_enabled = false;
};

struct AxisLimitConfig {
    float min_deg = 0.0f;
    float max_deg = 0.0f;
};

struct AxisVelocityLimitConfig {
    float min_deg_s = 0.0f;
    float max_deg_s = 0.0f;
};

struct AxisCalibrationConfig {
    float zero_offset_deg = 0.0f;
    float home_offset_deg = 0.0f;
    bool inverted = false;
};

struct GeoLimitConfig {
    double min_value = 0.0;
    double max_value = 0.0;
};

struct PidConfig {
    float kp = 1.0f;
    float ki = 0.0f;
    float kd = 0.0f;
    float feed_forward = 0.0f;
    float max_integral = 10.0f;
    float max_output = 100.0f;
};

struct MotionConfig {
    float max_velocity_dps = 60.0f;
    float max_acceleration_dps2 = 30.0f;
};

struct PersistentConfig {
    static constexpr std::uint32_t kVersion = 3;

    std::uint32_t version = kVersion;
    bool has_location = false;
    double location_longitude_deg = 0.0;
    double location_latitude_deg = 0.0;
    double location_altitude_m = 0.0;
    ace::communication::ModeId startup_mode = ace::communication::ModeId::MANUAL;
    AxisLimitConfig pan_angle_limit {-180.0f, 180.0f};
    AxisLimitConfig tilt_angle_limit {-90.0f, 90.0f};
    AxisVelocityLimitConfig pan_velocity_limit {-60.0f, 60.0f};
    AxisVelocityLimitConfig tilt_velocity_limit {-60.0f, 60.0f};
    GeoLimitConfig longitude_limit {-180.0, 180.0};
    GeoLimitConfig latitude_limit {-90.0, 90.0};
    GeoLimitConfig altitude_limit {-500.0, 9000.0};
    GeoLimitConfig target_longitude_limit {-180.0, 180.0};
    GeoLimitConfig target_latitude_limit {-90.0, 90.0};
    GeoLimitConfig target_altitude_limit {-500.0, 9000.0};
    float position_tolerance_deg = 0.25f;
    float velocity_tolerance_deg_s = 0.25f;
    double target_tolerance_m = 1.0;
    std::uint32_t motion_timeout_ms = 5000;
    std::uint32_t home_timeout_ms = 10000;
    std::uint32_t calibration_timeout_ms = 15000;
    AxisCalibrationConfig pan_calibration {};
    AxisCalibrationConfig tilt_calibration {};
    PidConfig pan_pid {};
    PidConfig tilt_pid {};
    MotionConfig pan_motion {};
    MotionConfig tilt_motion {};
};

}  // namespace ace::config
