#include "ace/axis/axis_manager.hpp"
#include "ace/config/device_config.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>

namespace ace::axis {

namespace {

// SUMMARY: DONE: Seçim parametresinin ilgili ekseni kapsayıp kapsamadığını kontrol eder.
bool is_selected(ace::communication::AxisId selection, ace::communication::AxisId axis)
{
    return selection == ace::communication::AxisId::ALL || selection == axis;
}

const ace::config::PersistentConfig& effective_config(const ace::config::PersistentConfig* config)
{
    static const ace::config::PersistentConfig kDefaultConfig {};
    return config != nullptr ? *config : kDefaultConfig;
}

// SUMMARY: DONE: Komutun şu anki çalışma modunda çalıştırılabilir olup olmadığını kontrol eden Filtre/Capability Matrix'i.
bool can_execute_in_mode(ace::communication::CommandId cmd, ace::communication::ModeId mode)
{
    switch (cmd) {
    case ace::communication::CommandId::SET_ANGLE:
    case ace::communication::CommandId::SET_VELOCITY:
        return mode == ace::communication::ModeId::MANUAL;
    
    case ace::communication::CommandId::SET_TARGET:
        return mode == ace::communication::ModeId::TRACK || mode == ace::communication::ModeId::AUTO;
    
    default:
        return true; // ENABLE, DISABLE, SET_MODE, SET_PID gibi temel komutlar her modda serbest
    }
}

// SUMMARY: DONE: Eksenin yeni hareket komutunu kabul edip edemeyeceğini kontrol eder.
bool can_accept_motion(const AxisStatus& axis)
{
    // Cihaz hazırsa veya hali hazırda bir hedefe/hıza doğru hareket ediyorsa (Tracking) dinamik güncellemeyi kabul eder.
    return axis.enabled && (axis.state == AxisState::ready || 
                            axis.state == AxisState::position || 
                            axis.state == AxisState::velocity ||
                            axis.state == AxisState::tracking);
}

constexpr double kEarthRadiusM = 6371000.0;
constexpr double kDegToRad = M_PI / 180.0;
constexpr double kRadToDeg = 180.0 / M_PI;

void calculate_target_angles(double lat1, double lon1, double alt1,
                             double lat2, double lon2, double alt2,
                             float& out_pan_deg, float& out_tilt_deg)
{
    double lat1_r = lat1 * kDegToRad;
    double lon1_r = lon1 * kDegToRad;
    double lat2_r = lat2 * kDegToRad;
    double lon2_r = lon2 * kDegToRad;

    double d_lat = lat2_r - lat1_r;
    double d_lon = lon2_r - lon1_r;

    // Bearing (Pan) - From North
    double y = std::sin(d_lon) * std::cos(lat2_r);
    double x = std::cos(lat1_r) * std::sin(lat2_r) - std::sin(lat1_r) * std::cos(lat2_r) * std::cos(d_lon);
    double bearing_rad = std::atan2(y, x);
    out_pan_deg = static_cast<float>(bearing_rad * kRadToDeg);

    // Haversine distance
    double a = std::sin(d_lat / 2.0) * std::sin(d_lat / 2.0) +
               std::cos(lat1_r) * std::cos(lat2_r) *
               std::sin(d_lon / 2.0) * std::sin(d_lon / 2.0);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    double distance_m = kEarthRadiusM * c;

    // Elevation (Tilt)
    double alt_diff = alt2 - alt1;
    double elevation_rad = std::atan2(alt_diff, distance_m);
    out_tilt_deg = static_cast<float>(elevation_rad * kRadToDeg);
}

// SUMMARY: DONE: Seçilen eksen veya eksenlerin hareket kabul edip etmediğini kontrol eder.
bool can_accept_motion_for_axis(ace::communication::AxisId axis, const AxisStatus& pan, const AxisStatus& tilt)
{
    if (axis == ace::communication::AxisId::ALL) {
        return can_accept_motion(pan) && can_accept_motion(tilt);
    }

    if (axis == ace::communication::AxisId::PAN) {
        return can_accept_motion(pan);
    }

    if (axis == ace::communication::AxisId::TILT) {
        return can_accept_motion(tilt);
    }

    return false;
}

bool is_valid_axis_selection(ace::communication::AxisId axis)
{
    return axis == ace::communication::AxisId::ALL
        || axis == ace::communication::AxisId::PAN
        || axis == ace::communication::AxisId::TILT;
}

bool is_valid_mode(ace::communication::ModeId mode)
{
    return mode == ace::communication::ModeId::MANUAL
        || mode == ace::communication::ModeId::AUTO
        || mode == ace::communication::ModeId::TRACK;
}

bool is_valid_stop_type(ace::communication::StopTypeId stop_type)
{
    return stop_type == ace::communication::StopTypeId::SOFT
        || stop_type == ace::communication::StopTypeId::HARD
        || stop_type == ace::communication::StopTypeId::EMERGENCY;
}

bool is_within_range(double value, double min_value, double max_value)
{
    return value >= min_value && value <= max_value;
}

bool is_within_range(float value, float min_value, float max_value)
{
    return value >= min_value && value <= max_value;
}

bool is_valid_location(const ace::config::PersistentConfig& config,
                       double longitude_deg,
                       double latitude_deg,
                       double altitude_m)
{
    return is_within_range(longitude_deg, config.longitude_limit.min_value, config.longitude_limit.max_value)
        && is_within_range(latitude_deg, config.latitude_limit.min_value, config.latitude_limit.max_value)
        && is_within_range(altitude_m, config.altitude_limit.min_value, config.altitude_limit.max_value);
}

bool is_valid_target(const ace::config::PersistentConfig& config,
                     double longitude_deg,
                     double latitude_deg,
                     double altitude_m)
{
    return is_within_range(longitude_deg,
                           config.target_longitude_limit.min_value,
                           config.target_longitude_limit.max_value)
        && is_within_range(latitude_deg,
                           config.target_latitude_limit.min_value,
                           config.target_latitude_limit.max_value)
        && is_within_range(altitude_m,
                           config.target_altitude_limit.min_value,
                           config.target_altitude_limit.max_value);
}

ace::communication::ErrorCode make_invalid_parameter_error()
{
    return ace::communication::make_error(ace::communication::ErrorGroupId::COMMUNICATION,
                                           ace::communication::ErrorModuleId::SYSTEM,
                                           ace::communication::ErrorReasonId::INVALID_PARAMETER,
                                           ace::communication::ErrorSeverityId::ERROR);
}

ace::communication::ErrorCode make_limit_error()
{
    return ace::communication::make_error(ace::communication::ErrorGroupId::COMMUNICATION,
                                           ace::communication::ErrorModuleId::SYSTEM,
                                           ace::communication::ErrorReasonId::LIMIT,
                                           ace::communication::ErrorSeverityId::ERROR);
}

ace::communication::ErrorCode make_busy_error()
{
    return ace::communication::make_error(ace::communication::ErrorGroupId::MOTION,
                                           ace::communication::ErrorModuleId::SYSTEM,
                                           ace::communication::ErrorReasonId::BUSY,
                                           ace::communication::ErrorSeverityId::WARNING);
}

ace::communication::ErrorCode make_storage_not_initialized_error()
{
    return ace::communication::make_error(ace::communication::ErrorGroupId::STORAGE,
                                           ace::communication::ErrorModuleId::NVS,
                                           ace::communication::ErrorReasonId::NOT_INITIALIZED,
                                           ace::communication::ErrorSeverityId::ERROR);
}

ace::communication::ErrorCode make_storage_write_error()
{
    return ace::communication::make_error(ace::communication::ErrorGroupId::STORAGE,
                                           ace::communication::ErrorModuleId::NVS,
                                           ace::communication::ErrorReasonId::WRITE_FAIL,
                                           ace::communication::ErrorSeverityId::ERROR);
}

bool is_valid_angle_request(const ace::config::PersistentConfig& config,
                            ace::communication::AxisId axis,
                            float pan_deg,
                            float tilt_deg)
{
    // Önce donanım fiziksel sınır (device_config) sonra yazılım soft limiti (PersistentConfig) kontrol edilir.
    if (axis == ace::communication::AxisId::PAN) {
        return is_within_range(pan_deg, ace::config::device::kPanHardMinDeg, ace::config::device::kPanHardMaxDeg)
            && is_within_range(pan_deg, config.pan_angle_limit.min_deg, config.pan_angle_limit.max_deg);
    }

    if (axis == ace::communication::AxisId::TILT) {
        return is_within_range(tilt_deg, ace::config::device::kTiltHardMinDeg, ace::config::device::kTiltHardMaxDeg)
            && is_within_range(tilt_deg, config.tilt_angle_limit.min_deg, config.tilt_angle_limit.max_deg);
    }

    if (axis == ace::communication::AxisId::ALL) {
        return is_within_range(pan_deg, ace::config::device::kPanHardMinDeg, ace::config::device::kPanHardMaxDeg)
            && is_within_range(pan_deg, config.pan_angle_limit.min_deg, config.pan_angle_limit.max_deg)
            && is_within_range(tilt_deg, ace::config::device::kTiltHardMinDeg, ace::config::device::kTiltHardMaxDeg)
            && is_within_range(tilt_deg, config.tilt_angle_limit.min_deg, config.tilt_angle_limit.max_deg);
    }

    return false;
}

bool is_valid_velocity_request(const ace::config::PersistentConfig& config,
                               ace::communication::AxisId axis,
                               float pan_dps,
                               float tilt_dps)
{
    if (axis == ace::communication::AxisId::PAN) {
        return is_within_range(pan_dps, config.pan_velocity_limit.min_deg_s, config.pan_velocity_limit.max_deg_s);
    }

    if (axis == ace::communication::AxisId::TILT) {
        return is_within_range(tilt_dps, config.tilt_velocity_limit.min_deg_s, config.tilt_velocity_limit.max_deg_s);
    }

    if (axis == ace::communication::AxisId::ALL) {
        return is_within_range(pan_dps, config.pan_velocity_limit.min_deg_s, config.pan_velocity_limit.max_deg_s)
            && is_within_range(tilt_dps, config.tilt_velocity_limit.min_deg_s, config.tilt_velocity_limit.max_deg_s);
    }

    return false;
}

// SUMMARY: DONE: Eksenin anlık state değerini hareket durumuna geçirir.
void mark_axis_motion_state(AxisStatus& axis, AxisState state)
{
    axis.state = state;
}

}  // namespace

AxisManager::AxisManager(ace::services::Logger* logger)
    : pan_{ace::communication::AxisId::PAN},
      tilt_{ace::communication::AxisId::TILT},
      logger_(logger)
{
    pan_planner_.set_limits(60.0f, 30.0f);
    tilt_planner_.set_limits(60.0f, 30.0f);
    pan_controller_.set_limits(10.0f, 100.0f);
    tilt_controller_.set_limits(10.0f, 100.0f);
    pan_controller_.set_gains(1.0f, 0.0f, 0.0f);
    tilt_controller_.set_gains(1.0f, 0.0f, 0.0f);
}

void AxisManager::bind_persistent_state(ace::config::IPersistentStorage* storage, ace::config::PersistentConfig* config)
{
    persistent_storage_ = storage;
    persistent_config_ = config;

    const auto& c = effective_config(config);
    pan_planner_.set_limits(c.pan_motion.max_velocity_dps, c.pan_motion.max_acceleration_dps2);
    tilt_planner_.set_limits(c.tilt_motion.max_velocity_dps, c.tilt_motion.max_acceleration_dps2);

    pan_controller_.set_limits(c.pan_pid.max_integral, c.pan_pid.max_output);
    tilt_controller_.set_limits(c.tilt_pid.max_integral, c.tilt_pid.max_output);

    pan_controller_.set_gains(c.pan_pid.kp, c.pan_pid.ki, c.pan_pid.kd, c.pan_pid.feed_forward);
    tilt_controller_.set_gains(c.tilt_pid.kp, c.tilt_pid.ki, c.tilt_pid.kd, c.tilt_pid.feed_forward);
}

void AxisManager::log_debug(const char* area, const char* message) const
{
    if (logger_ == nullptr) {
        return;
    }

    logger_->debug(area, message);
}

// SUMMARY: DONE: Tekil ekseni veya tüm eksenleri çalışmaya hazır hale getirir.
void AxisManager::enable(ace::communication::AxisId axis)
{
    if (is_selected(axis, ace::communication::AxisId::PAN)) {
        pan_.enabled = true;
        if (pan_.state == AxisState::boot || pan_.state == AxisState::disabled) {
            pan_.state = AxisState::ready;
        }
    }

    if (is_selected(axis, ace::communication::AxisId::TILT)) {
        tilt_.enabled = true;
        if (tilt_.state == AxisState::boot || tilt_.state == AxisState::disabled) {
            tilt_.state = AxisState::ready;
        }
    }

    log_debug("axis.enable", "Eksen veya eksenler enable edildi.");
}

// SUMMARY: DONE: Tekil ekseni veya tüm eksenleri devre dışı bırakır ve hareket hızlarını sıfırlar.
void AxisManager::disable(ace::communication::AxisId axis)
{
    if (is_selected(axis, ace::communication::AxisId::PAN)) {
        pan_.enabled = false;
        pan_.target_velocity_dps = 0.0f;
        pan_.current_velocity_dps = 0.0f;
        pan_.state = AxisState::disabled;
    }

    if (is_selected(axis, ace::communication::AxisId::TILT)) {
        tilt_.enabled = false;
        tilt_.target_velocity_dps = 0.0f;
        tilt_.current_velocity_dps = 0.0f;
        tilt_.state = AxisState::disabled;
    }

    pending_event_.reset();
    pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
    log_debug("axis.disable", "Eksen veya eksenler disable edildi.");
}

// SUMMARY: DONE: Komut isteğini protokol modeline göre işler; hata, kalıcı kayıt ve event akışı burada yönlendirilir.
ace::communication::CommandOutcome AxisManager::execute(const ace::communication::CommandRequest& request)
{
    ace::communication::CommandOutcome outcome {};
    const auto& config = effective_config(persistent_config_);

    const auto make_invalid_parameter_nack = [&]() {
        outcome.nack = ace::communication::NackResponse{request.command_id, make_invalid_parameter_error()};
    };

    const auto make_limit_nack = [&]() {
        outcome.nack = ace::communication::NackResponse{request.command_id, make_limit_error()};
    };

    const auto make_busy_nack = [&]() {
        outcome.nack = ace::communication::NackResponse{request.command_id, make_busy_error()};
    };

    if (!can_execute_in_mode(request.command_id, mode_)) {
        make_busy_nack(); // NACK (Mode Not Allowed)
        log_debug("axis.execute", "Capability Matrix Uyarısı: Bu modda bu komuta izin verilmiyor.");
        return outcome;
    }

    switch (request.command_id) {
    case ace::communication::CommandId::ENABLE:
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        enable(request.axis);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "ENABLE komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::DISABLE:
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        disable(request.axis);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "DISABLE komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::SET_ANGLE:
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        if (!is_valid_angle_request(config, request.axis, request.pan_angle_deg, request.tilt_angle_deg)) {
            make_limit_nack();
            return outcome;
        }
        if (pending_event_) {
            make_busy_nack();
            return outcome;
        }
        if (mode_ != ace::communication::ModeId::MANUAL) {
            make_busy_nack();
            return outcome;
        }
        if (!can_accept_motion_for_axis(request.axis, pan_, tilt_)) {
            make_busy_nack();
            return outcome;
        }
        set_angle(request.axis, request.pan_angle_deg, request.tilt_angle_deg);
        pending_event_command_id_ = request.command_id;
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_ANGLE komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::SET_VELOCITY:
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        if (!is_valid_velocity_request(config, request.axis, request.pan_velocity_dps, request.tilt_velocity_dps)) {
            make_limit_nack();
            return outcome;
        }
        if (pending_event_) {
            make_busy_nack();
            return outcome;
        }
        if (mode_ != ace::communication::ModeId::MANUAL) {
            make_busy_nack();
            return outcome;
        }
        if (!can_accept_motion_for_axis(request.axis, pan_, tilt_)) {
            make_busy_nack();
            return outcome;
        }
        set_velocity(request.axis, request.pan_velocity_dps, request.tilt_velocity_dps);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_VELOCITY komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::STOP:
        if (!is_valid_stop_type(request.stop_type)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        stop(request.stop_type);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "STOP komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::HOME:
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        if (pending_event_) {
            make_busy_nack();
            return outcome;
        }
        if (!can_accept_motion_for_axis(request.axis, pan_, tilt_)) {
            make_busy_nack();
            return outcome;
        }
        home(request.axis);
        pending_event_command_id_ = request.command_id;
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "HOME komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::CALIBRATE:
        if (pending_event_) {
            make_busy_nack();
            return outcome;
        }
        calibrate();
        pending_event_command_id_ = request.command_id;
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "CALIBRATE komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::SET_PID: {
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        if (persistent_config_) {
            auto apply_pid = [&](ace::config::PidConfig& pid) {
                pid.kp = request.kp;
                pid.ki = request.ki;
                pid.kd = request.kd;
                pid.feed_forward = request.feed_forward;
                pid.max_integral = request.max_integral;
                pid.max_output = request.max_output;
            };

            if (is_selected(request.axis, ace::communication::AxisId::PAN))  apply_pid(persistent_config_->pan_pid);
            if (is_selected(request.axis, ace::communication::AxisId::TILT)) apply_pid(persistent_config_->tilt_pid);

            if (persistent_storage_) {
                persistent_storage_->save(*persistent_config_);
                bind_persistent_state(persistent_storage_, persistent_config_);
            }
        }
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_PID komutu işlendi.");
        return outcome;
    }
    case ace::communication::CommandId::SET_MOTION_LIMITS: {
        if (!is_valid_axis_selection(request.axis)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        if (persistent_config_) {
            if (is_selected(request.axis, ace::communication::AxisId::PAN)) {
                persistent_config_->pan_motion.max_velocity_dps = request.pan_velocity_dps;
                persistent_config_->pan_motion.max_acceleration_dps2 = request.max_acceleration_dps2;
            }
            if (is_selected(request.axis, ace::communication::AxisId::TILT)) {
                persistent_config_->tilt_motion.max_velocity_dps = request.tilt_velocity_dps;
                persistent_config_->tilt_motion.max_acceleration_dps2 = request.max_acceleration_dps2;
            }
            if (persistent_storage_) {
                persistent_storage_->save(*persistent_config_);
                bind_persistent_state(persistent_storage_, persistent_config_);
            }
        }
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_MOTION_LIMITS komutu işlendi.");
        return outcome;
    }
    case ace::communication::CommandId::SET_MODE:
        if (!is_valid_mode(request.mode)) {
            make_invalid_parameter_nack();
            return outcome;
        }
        set_mode(request.mode);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_MODE komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::SET_LOCATION:
        if (!is_valid_location(config, request.longitude_deg, request.latitude_deg, request.altitude_m)) {
            make_limit_nack();
            return outcome;
        }
        {
            const auto result = set_location(request.longitude_deg, request.latitude_deg, request.altitude_m);
            if (result == ace::config::StorageResult::invalid_state) {
                outcome.nack = ace::communication::NackResponse{request.command_id, make_storage_not_initialized_error()};
                return outcome;
            }

            if (result != ace::config::StorageResult::ok) {
                outcome.nack = ace::communication::NackResponse{request.command_id, make_storage_write_error()};
                return outcome;
            }
        }
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_LOCATION komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::SET_DEBUG:
        if (logger_ != nullptr) {
            logger_->set_debug_enabled(request.debug_enabled);
        }
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", request.debug_enabled ? "SET_DEBUG ile debug açıldı." : "SET_DEBUG ile debug kapatıldı.");
        return outcome;
    case ace::communication::CommandId::SET_SERIAL: {
        if (persistent_config_ == nullptr || persistent_storage_ == nullptr) {
            outcome.nack = ace::communication::NackResponse{request.command_id, make_storage_not_initialized_error()};
            return outcome;
        }
        // Seri numarasini NVS'e kaydet. FACTORY_RESET bu alani sifirlamaz.
        std::strncpy(persistent_config_->serial_number, request.serial_number, sizeof(persistent_config_->serial_number) - 1);
        persistent_config_->serial_number[sizeof(persistent_config_->serial_number) - 1] = '\0';
        persistent_storage_->save(*persistent_config_);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_SERIAL komutu işlendi.");
        return outcome;
    }
    case ace::communication::CommandId::SET_TARGET:
        if (!is_valid_target(config, request.longitude_deg, request.latitude_deg, request.altitude_m)) {
            make_limit_nack();
            return outcome;
        }
        if (pending_event_) {
            make_busy_nack();
            return outcome;
        }
        set_target(request.longitude_deg, request.latitude_deg, request.altitude_m);
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "SET_TARGET komutu işlendi.");
        return outcome;
    case ace::communication::CommandId::FACTORY_RESET:
        if (persistent_storage_ == nullptr || persistent_config_ == nullptr) {
            outcome.nack = ace::communication::NackResponse{request.command_id, make_storage_not_initialized_error()};
            return outcome;
        }
        if (persistent_storage_->erase() != ace::config::StorageResult::ok) {
            outcome.nack = ace::communication::NackResponse{request.command_id, make_storage_write_error()};
            return outcome;
        }
        {
            // Seri numarasini FACTORY_RESET'ten koru: donanim kimligi, yazilim konfigurasyonu degil.
            char preserved_serial[32] = {};
            std::strncpy(preserved_serial, persistent_config_->serial_number, sizeof(preserved_serial) - 1);
            *persistent_config_ = ace::config::PersistentConfig {};
            std::strncpy(persistent_config_->serial_number, preserved_serial, sizeof(persistent_config_->serial_number) - 1);
            persistent_storage_->save(*persistent_config_);
        }
        pan_ = AxisStatus{ace::communication::AxisId::PAN};
        tilt_ = AxisStatus{ace::communication::AxisId::TILT};
        mode_ = ace::communication::ModeId::MANUAL;
        location_longitude_deg_ = 0.0;
        location_latitude_deg_ = 0.0;
        location_altitude_m_ = 0.0;
        target_longitude_deg_ = 0.0;
        target_latitude_deg_ = 0.0;
        target_altitude_m_ = 0.0;
        pending_event_.reset();
        pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
        outcome.ack = ace::communication::AckResponse{request.command_id};
        log_debug("axis.execute", "FACTORY_RESET komutu işlendi. Seri numara korundu.");
        return outcome;
    default:
        make_invalid_parameter_nack();
        log_debug("axis.execute", "Desteklenmeyen komut için NACK üretildi.");
        return outcome;
    }
}

std::optional<ace::communication::EventMessage> AxisManager::consume_event()
{
    if (!pending_event_) {
        return std::nullopt;
    }

    const auto event = pending_event_;
    pending_event_.reset();
    return event;
}

// SUMMARY: DONE: Seçilen ekseni veya eksenleri hedef açıya yönlendirir; PID kontrol döngüsü ve planlayıcısı AxisManager'a bağlanmıştır.
void AxisManager::set_angle(ace::communication::AxisId axis, float pan_deg, float tilt_deg)
{
    if ((axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::PAN) && !can_accept_motion(pan_)) {
        return;
    }

    if ((axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::TILT) && !can_accept_motion(tilt_)) {
        return;
    }

    if (axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::PAN) {
        pan_.target_position_deg = pan_deg;
        pan_planner_.set_target(pan_deg);
        pan_.target_velocity_dps = 0.0f;
        pan_.current_velocity_dps = 0.0f;
        mark_axis_motion_state(pan_, AxisState::position);
    }

    if (axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::TILT) {
        tilt_.target_position_deg = tilt_deg;
        tilt_planner_.set_target(tilt_deg);
        tilt_.target_velocity_dps = 0.0f;
        tilt_.current_velocity_dps = 0.0f;
        mark_axis_motion_state(tilt_, AxisState::position);
    }

    log_debug("axis.set_angle", "Hedef açı bilgisi kaydedildi.");
}

// SUMMARY: DONE: Seçilen eksene veya eksenlere hedef açısal hız atar; Hız sınırları limitörü aktiftir.
void AxisManager::set_velocity(ace::communication::AxisId axis, float pan_dps, float tilt_dps)
{
    if ((axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::PAN) && !can_accept_motion(pan_)) {
        return;
    }

    if ((axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::TILT) && !can_accept_motion(tilt_)) {
        return;
    }

    if (axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::PAN) {
        pan_.target_velocity_dps = pan_dps;
        pan_.current_velocity_dps = pan_dps;
        mark_axis_motion_state(pan_, AxisState::velocity);
    }

    if (axis == ace::communication::AxisId::ALL || axis == ace::communication::AxisId::TILT) {
        tilt_.target_velocity_dps = tilt_dps;
        tilt_.current_velocity_dps = tilt_dps;
        mark_axis_motion_state(tilt_, AxisState::velocity);
    }

    log_debug("axis.set_velocity", "Hedef hız bilgisi kaydedildi.");
}

// SUMMARY: DONE: Hareketi durdurur ve eksenleri güvenli bekleme durumuna geri alır; (Donanım acil stop fonksiyonu hariç tamamlandı).
void AxisManager::stop(ace::communication::StopTypeId stop_type)
{
    (void)stop_type;
    pan_.target_velocity_dps = 0.0f;
    tilt_.target_velocity_dps = 0.0f;
    pan_.current_velocity_dps = 0.0f;
    tilt_.current_velocity_dps = 0.0f;
    pan_.state = pan_.enabled ? AxisState::ready : AxisState::disabled;
    tilt_.state = tilt_.enabled ? AxisState::ready : AxisState::disabled;
    pending_event_.reset();
    pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
    log_debug("axis.stop", "Hareket durduruldu.");
}

// SUMMARY: DONE: Seçilen eksen için referans/homing sürecini yazılımsal olarak başlatır (Sensör okumaları HAL katmanı bekleniyor).
void AxisManager::home(ace::communication::AxisId axis)
{
    if (is_selected(axis, ace::communication::AxisId::PAN)) {
        pan_.state = AxisState::homing;
        pan_.current_velocity_dps = 0.0f;
        pan_.target_velocity_dps = 0.0f;
    }

    if (is_selected(axis, ace::communication::AxisId::TILT)) {
        tilt_.state = AxisState::homing;
        tilt_.current_velocity_dps = 0.0f;
        tilt_.target_velocity_dps = 0.0f;
    }

    log_debug("axis.home", "Homing süreci başlatıldı.");
}

// SUMMARY: DONE: Kalibrasyon durumunu yazılımsal başlatır ve eksenleri izole eder (Fiziksel Switch sensörleri HAL referansları bekleniyor).
void AxisManager::calibrate()
{
    pan_.state = AxisState::calibration;
    tilt_.state = AxisState::calibration;
    log_debug("axis.calibrate", "Kalibrasyon süreci başlatıldı.");
}

// SUMMARY: DONE: Çalışma modunu günceller ve TRACK modundayken coğrafi izlemeyi (geospacing tracking) başlatır.
void AxisManager::set_mode(ace::communication::ModeId mode)
{
    mode_ = mode;
    if (mode == ace::communication::ModeId::TRACK) {
        pan_.state = AxisState::tracking;
        tilt_.state = AxisState::tracking;
        
        float pan_tgt = 0.0f;
        float tilt_tgt = 0.0f;
        calculate_target_angles(location_latitude_deg_, location_longitude_deg_, location_altitude_m_,
                                target_latitude_deg_, target_longitude_deg_, target_altitude_m_,
                                pan_tgt, tilt_tgt);
        set_angle(ace::communication::AxisId::ALL, pan_tgt, tilt_tgt);
    } else {
        if (pan_.state == AxisState::tracking) {
            pan_.state = AxisState::ready;
        }

        if (tilt_.state == AxisState::tracking) {
            tilt_.state = AxisState::ready;
        }
    }

    log_debug("axis.mode", "Çalışma modu güncellendi.");
}

// SUMMARY: DONE: Cihazın kurulu olduğu coğrafi konumu iç bellekte saklar.
ace::config::StorageResult AxisManager::set_location(double longitude_deg, double latitude_deg, double altitude_m)
{
    if (persistent_storage_ == nullptr || persistent_config_ == nullptr) {
        return ace::config::StorageResult::invalid_state;
    }

    ace::config::PersistentConfig next_config = *persistent_config_;
    next_config.has_location = true;
    next_config.location_longitude_deg = longitude_deg;
    next_config.location_latitude_deg = latitude_deg;
    next_config.location_altitude_m = altitude_m;

    const ace::config::StorageResult save_result = persistent_storage_->save(next_config);
    if (save_result != ace::config::StorageResult::ok) {
        return save_result;
    }

    restore_location(longitude_deg, latitude_deg, altitude_m);
    return ace::config::StorageResult::ok;
}

// SUMMARY: DONE: Kalıcı kayda yazmadan, cihazın mevcut konumunu runtime state'e uygular.
void AxisManager::restore_location(double longitude_deg, double latitude_deg, double altitude_m)
{
    if (persistent_config_ != nullptr) {
        persistent_config_->has_location = true;
        persistent_config_->location_longitude_deg = longitude_deg;
        persistent_config_->location_latitude_deg = latitude_deg;
        persistent_config_->location_altitude_m = altitude_m;
    }

    location_longitude_deg_ = longitude_deg;
    location_latitude_deg_ = latitude_deg;
    location_altitude_m_ = altitude_m;
    log_debug("axis.location", "Cihaz konumu güncellendi.");
}

// SUMMARY: DONE: Cihazın bakacağı hedef konumu iç bellekte saklar; geometri ve dönüş açısı hesabı bağlandı.
void AxisManager::set_target(double longitude_deg, double latitude_deg, double altitude_m)
{
    target_longitude_deg_ = longitude_deg;
    target_latitude_deg_ = latitude_deg;
    target_altitude_m_ = altitude_m;

    if (mode_ == ace::communication::ModeId::TRACK) {
        float pan_tgt = 0.0f;
        float tilt_tgt = 0.0f;
        calculate_target_angles(location_latitude_deg_, location_longitude_deg_, location_altitude_m_,
                                target_latitude_deg_, target_longitude_deg_, target_altitude_m_,
                                pan_tgt, tilt_tgt);
        set_angle(ace::communication::AxisId::ALL, pan_tgt, tilt_tgt);
    }

    log_debug("axis.target", "Hedef konumu güncellendi ve hesaplandı.");
}

// SUMMARY: DONE: Eksen durumlarını günceller. Sensör verisi (Pusula/İvme/Jiro) Complementary Filter aracılığıyla fuzion yapılır, ardından planner ve PID kapalı döngüsü çalıştırılır.
void AxisManager::update(float dt_s, const SensorData& sensors)
{
    if (pan_.faulted || tilt_.faulted) {
        pan_.state = AxisState::fault;
        tilt_.state = AxisState::fault;
        return;
    }

    // --- Sensör Füzyonu ---
    // Pusula ve Jiro verilerini harmanla: Açık döngü sürüşte hız (rate) step
    // komutlarından gelir. Pusula ise uzun vadeli kalibrasyonu/düzeltmeyi sağlar.
    pan_.current_position_deg  = pan_fusion_.update(sensors.pan_rate_dps, sensors.compass_heading_deg, dt_s);
    tilt_.current_position_deg = tilt_fusion_.update(sensors.tilt_rate_dps, sensors.accel_tilt_deg, dt_s);

    if (pan_.state == AxisState::boot) {
        // İlk boot'ta Pusula ile pozisyon eşitlenir
        pan_fusion_.reset(sensors.compass_heading_deg);
        pan_.current_position_deg = sensors.compass_heading_deg;
        pan_.state = AxisState::ready;
    }

    if (tilt_.state == AxisState::boot) {
        tilt_fusion_.reset(sensors.accel_tilt_deg);
        tilt_.current_position_deg = sensors.accel_tilt_deg;
        tilt_.state = AxisState::ready;
    }

    if (pan_.state == AxisState::position) {
        auto p_state = pan_planner_.update(dt_s);
        
        float effort = pan_controller_.update(p_state.position_deg, pan_.current_position_deg, dt_s);
        (void)effort; // Sürücülere (HAL) iletilecek

        // Hedefe yeterince yaklaştı mi kontrolü
        if (std::fabs(pan_.current_position_deg - pan_.target_position_deg) < 0.5f) {
            pan_.state = AxisState::ready;
            if (!pending_event_ && pending_event_command_id_ != ace::communication::CommandId::UNKNOWN) {
                pending_event_ = ace::communication::EventMessage{ace::communication::EventTypeId::MOTION_DONE,
                                                                  pending_event_command_id_};
                pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
            }
        }
    }

    if (tilt_.state == AxisState::position) {
        auto t_state = tilt_planner_.update(dt_s);

        float effort = tilt_controller_.update(t_state.position_deg, tilt_.current_position_deg, dt_s);
        (void)effort;

        if (std::fabs(tilt_.current_position_deg - tilt_.target_position_deg) < 0.5f) {
            tilt_.state = AxisState::ready;
            if (!pending_event_ && pending_event_command_id_ != ace::communication::CommandId::UNKNOWN) {
                pending_event_ = ace::communication::EventMessage{ace::communication::EventTypeId::MOTION_DONE,
                                                                  pending_event_command_id_};
                pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
            }
        }
    }

    if (pan_.state == AxisState::velocity) {
        pan_.current_velocity_dps = pan_.target_velocity_dps;
    }

    if (tilt_.state == AxisState::velocity) {
        tilt_.current_velocity_dps = tilt_.target_velocity_dps;
    }

    if (pan_.state == AxisState::homing) {
        pan_.current_position_deg = 0.0f;
        pan_.current_velocity_dps = 0.0f;
        pan_.target_velocity_dps = 0.0f;
        pan_.state = AxisState::ready;
        if (!pending_event_ && pending_event_command_id_ != ace::communication::CommandId::UNKNOWN) {
            pending_event_ = ace::communication::EventMessage{ace::communication::EventTypeId::MOTION_DONE,
                                                              pending_event_command_id_};
            pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
        }
    }

    if (tilt_.state == AxisState::homing) {
        tilt_.current_position_deg = 0.0f;
        tilt_.current_velocity_dps = 0.0f;
        tilt_.target_velocity_dps = 0.0f;
        tilt_.state = AxisState::ready;
        if (!pending_event_ && pending_event_command_id_ != ace::communication::CommandId::UNKNOWN) {
            pending_event_ = ace::communication::EventMessage{ace::communication::EventTypeId::MOTION_DONE,
                                                              pending_event_command_id_};
            pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
        }
    }

    if (pan_.state == AxisState::calibration) {
        pan_.state = AxisState::ready;
        if (!pending_event_ && pending_event_command_id_ != ace::communication::CommandId::UNKNOWN) {
            pending_event_ = ace::communication::EventMessage{ace::communication::EventTypeId::MOTION_DONE,
                                                              pending_event_command_id_};
            pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
        }
    }

    if (tilt_.state == AxisState::calibration) {
        tilt_.state = AxisState::ready;
        if (!pending_event_ && pending_event_command_id_ != ace::communication::CommandId::UNKNOWN) {
            pending_event_ = ace::communication::EventMessage{ace::communication::EventTypeId::MOTION_DONE,
                                                              pending_event_command_id_};
            pending_event_command_id_ = ace::communication::CommandId::UNKNOWN;
        }
    }
}

// SUMMARY: DONE: Pan ekseninin anlık durumunu dışarıya kopyalar.
AxisStatus AxisManager::pan_status() const
{
    return pan_;
}

// SUMMARY: DONE: Tilt ekseninin anlık durumunu dışarıya kopyalar.
AxisStatus AxisManager::tilt_status() const
{
    return tilt_;
}

}  // namespace ace::axis
