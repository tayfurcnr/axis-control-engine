#pragma once

#include <cstdint>

#include "ace/communication/message_models.hpp"
#include "ace/control/controller.hpp"
#include "ace/motion/motion_planner.hpp"
#include "ace/config/config.hpp"
#include "ace/config/persistent_storage_interface.hpp"
#include "ace/services/logger.hpp"
#include "ace/control/sensor_fusion.hpp"

namespace ace::axis {

struct SensorData {
    float pan_rate_dps = 0.0f;           // High freq velocity from Gyro or Stepper Pulses
    float tilt_rate_dps = 0.0f;          // High freq velocity from Gyro or Stepper Pulses
    float compass_heading_deg = 0.0f;    // Absolute Low Freq Pan reference (Magnetometer)
    float accel_tilt_deg = 0.0f;         // Absolute Low Freq Tilt reference (Accelerometer)
    // --- kHasEncoder = true durumunda doldurulur ---
    float encoder_pan_deg = 0.0f;        // Optik/Manyetik enkoder mutlak Pan açısı
    float encoder_tilt_deg = 0.0f;       // Optik/Manyetik enkoder mutlak Tilt açısı
};

// Logical runtime state only. No hardware handles live here.
enum class AxisState : std::uint8_t {
    boot,
    disabled,
    ready,
    position,
    velocity,
    tracking,
    homing,
    calibration,
    fault,
};

struct AxisStatus {
    ace::communication::AxisId id = ace::communication::AxisId::PAN;
    AxisState state = AxisState::boot;
    bool enabled = false;
    bool faulted = false;
    float target_position_deg = 0.0f;
    float target_velocity_dps = 0.0f;
    float current_position_deg = 0.0f;
    float current_velocity_dps = 0.0f;
};

class AxisManager {
public:
    explicit AxisManager(ace::services::Logger* logger = nullptr);

    void bind_persistent_state(ace::config::IPersistentStorage* storage, ace::config::PersistentConfig* config);

    void enable(ace::communication::AxisId axis);
    void disable(ace::communication::AxisId axis);
    [[nodiscard]] ace::communication::CommandOutcome execute(const ace::communication::CommandRequest& request);
    [[nodiscard]] std::optional<ace::communication::EventMessage> consume_event();
    void set_angle(ace::communication::AxisId axis, float pan_deg, float tilt_deg);
    void set_velocity(ace::communication::AxisId axis, float pan_dps, float tilt_dps);
    void stop(ace::communication::StopTypeId stop_type);
    void home(ace::communication::AxisId axis);
    void calibrate();
    void set_mode(ace::communication::ModeId mode);
    // TODO: Konum bilgisi, gerçek geodetik referans ve sensör kaynaklarıyla birlikte değerlendirilecek.
    [[nodiscard]] ace::config::StorageResult set_location(double longitude_deg, double latitude_deg, double altitude_m);
    // TODO: Kalıcı kayda dokunmadan, yüklenen konumu sadece runtime state'e uygular.
    void restore_location(double longitude_deg, double latitude_deg, double altitude_m);
    // TODO: Hedefe yönelim hesabı, cihaz konumu ve hedef koordinatlar kullanılarak ayrı bir geometri katmanına taşınacak.
    void set_target(double longitude_deg, double latitude_deg, double altitude_m);
    void update(float dt_s, const SensorData& sensors);

    [[nodiscard]] AxisStatus pan_status() const;
    [[nodiscard]] AxisStatus tilt_status() const;
    [[nodiscard]] ace::communication::ModeId mode() const { return mode_; }

private:
    void log_debug(const char* area, const char* message) const;

    AxisStatus pan_;
    AxisStatus tilt_;
    ace::communication::ModeId mode_ = ace::communication::ModeId::MANUAL;
    ace::services::Logger* logger_ = nullptr;
    ace::config::IPersistentStorage* persistent_storage_ = nullptr;
    ace::config::PersistentConfig* persistent_config_ = nullptr;
    std::optional<ace::communication::EventMessage> pending_event_ {};
    ace::communication::CommandId pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
    double location_longitude_deg_ = 0.0;
    double location_latitude_deg_ = 0.0;
    double location_altitude_m_ = 0.0;
    double target_longitude_deg_ = 0.0;
    double target_latitude_deg_ = 0.0;
    double target_altitude_m_ = 0.0;

    ace::motion::MotionPlanner pan_planner_;
    ace::control::Controller pan_controller_;
    ace::motion::MotionPlanner tilt_planner_;
    ace::control::Controller tilt_controller_;
    
    ace::control::ComplementaryFilter pan_fusion_;
    ace::control::ComplementaryFilter tilt_fusion_;
};

}  // namespace ace::axis
