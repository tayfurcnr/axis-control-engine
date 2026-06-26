#pragma once

#include <cstdint>

#include "ace/communication/command_ids.hpp"
#include "ace/config/device_config.hpp"

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
    // Cihaza ozgu benzersiz seri numarasi. FACTORY_RESET tarafindan SILINMEZ.
    // Sahada SET_SERIAL komutu ile yazilir.
    char serial_number[32] = {};
    double location_longitude_deg = 0.0;
    double location_latitude_deg = 0.0;
    double location_altitude_m = 0.0;
    ace::communication::ModeId startup_mode = ace::communication::ModeId::MANUAL;

    // Soft limits: operator tarafindan komutla guncellenebilir. Fabrika varsayilanlari device_config'den gelir.
    // Hard limits ise hic bir zaman asil amaz (donanim fiziginini temsil eder).
    AxisLimitConfig pan_angle_limit  {
        ace::config::device::kPanHardMinDeg,
        ace::config::device::kPanHardMaxDeg
    };
    AxisLimitConfig tilt_angle_limit {
        ace::config::device::kTiltHardMinDeg,
        ace::config::device::kTiltHardMaxDeg
    };
    AxisVelocityLimitConfig pan_velocity_limit  {-ace::config::device::kPanMaxVelocityDps,  ace::config::device::kPanMaxVelocityDps};
    AxisVelocityLimitConfig tilt_velocity_limit {-ace::config::device::kTiltMaxVelocityDps, ace::config::device::kTiltMaxVelocityDps};
    GeoLimitConfig longitude_limit {ace::config::device::kLongitudeMin, ace::config::device::kLongitudeMax};
    GeoLimitConfig latitude_limit  {ace::config::device::kLatitudeMin,  ace::config::device::kLatitudeMax};
    GeoLimitConfig altitude_limit  {ace::config::device::kAltitudeMin,  ace::config::device::kAltitudeMax};
    GeoLimitConfig target_longitude_limit {ace::config::device::kLongitudeMin, ace::config::device::kLongitudeMax};
    GeoLimitConfig target_latitude_limit  {ace::config::device::kLatitudeMin,  ace::config::device::kLatitudeMax};
    GeoLimitConfig target_altitude_limit  {ace::config::device::kAltitudeMin,  ace::config::device::kAltitudeMax};
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
    MotionConfig pan_motion  {ace::config::device::kPanMaxVelocityDps  / 2.0f, ace::config::device::kPanMaxAccelDps2  / 2.0f};
    MotionConfig tilt_motion {ace::config::device::kTiltMaxVelocityDps / 2.0f, ace::config::device::kTiltMaxAccelDps2 / 2.0f};
};

}  // namespace ace::config
