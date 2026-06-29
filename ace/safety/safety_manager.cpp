#include "ace/safety/safety_manager.hpp"

namespace ace::safety {

// -----------------------------------------------------------------------------
// Her hata kodunun önceden tanımlanmış ağırlığı (severity).
// -----------------------------------------------------------------------------
FaultSeverity SafetyManager::severity_of(FaultCode code)
{
    switch (code) {
    // WARNING — devam edilebilir
    case FaultCode::COMMAND_TIMEOUT:
    case FaultCode::INVALID_STATE:
    case FaultCode::OVER_TEMPERATURE:
    case FaultCode::UNDER_VOLTAGE:
    case FaultCode::IMU_UNAVAILABLE:
        return FaultSeverity::WARNING;

    // CRITICAL — eksenler durdurulur
    case FaultCode::AXIS_PAN_OVERRUN:
    case FaultCode::AXIS_TILT_OVERRUN:
    case FaultCode::OVER_CURRENT:
    case FaultCode::ENCODER_FAULT:
        return FaultSeverity::CRITICAL;

    // EMERGENCY — anında motor kes, sadece reboot ile çıkılır
    case FaultCode::EMERGENCY_STOP:
        return FaultSeverity::EMERGENCY;

    default:
        return FaultSeverity::WARNING;
    }
}

// -----------------------------------------------------------------------------
void SafetyManager::report_fault(FaultCode code)
{
    if (code == FaultCode::NONE) {
        return;
    }

    // Zaten kayıtlıysa sadece aktif et
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].code == code) {
            faults_[i].active = true;
            // Seviyeye göre safe state'e gir
            if (faults_[i].severity >= FaultSeverity::CRITICAL) {
                enter_safe_state();
            }
            return;
        }
    }

    // Yeni hata ekle (max kapasiteyi aşmamak kaydıyla)
    if (fault_count_ < kMaxFaults) {
        FaultEntry entry;
        entry.code     = code;
        entry.severity = severity_of(code);
        entry.active   = true;
        faults_[fault_count_++] = entry;

        if (entry.severity >= FaultSeverity::CRITICAL) {
            enter_safe_state();
        }
    }
}

// -----------------------------------------------------------------------------
bool SafetyManager::clear_fault(FaultCode code)
{
    // EMERGENCY_STOP asla yazılımla temizlenemez
    if (code == FaultCode::EMERGENCY_STOP) {
        return false;
    }

    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].code == code) {
            faults_[i].active = false;
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
void SafetyManager::clear_all()
{
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        // EMERGENCY_STOP korunur
        if (faults_[i].code != FaultCode::EMERGENCY_STOP) {
            faults_[i].active = false;
        }
    }
}

// -----------------------------------------------------------------------------
void SafetyManager::enter_safe_state()
{
    safe_state_active_ = true;
    // TODO (HAL entegrasyonunda): motor_driver_->disable() çağrılacak
}

// -----------------------------------------------------------------------------
bool SafetyManager::can_exit_safe_state() const
{
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].active && faults_[i].severity >= FaultSeverity::CRITICAL) {
            return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
bool SafetyManager::is_safe_state_active() const
{
    return safe_state_active_;
}

// -----------------------------------------------------------------------------
bool SafetyManager::has_fault() const
{
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].active) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
bool SafetyManager::has_fault(FaultCode code) const
{
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].code == code && faults_[i].active) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
std::optional<FaultEntry> SafetyManager::highest_active_fault() const
{
    std::optional<FaultEntry> result;
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].active) {
            if (!result || faults_[i].severity > result->severity) {
                result = faults_[i];
            }
        }
    }
    return result;
}

// -----------------------------------------------------------------------------
std::uint32_t SafetyManager::active_fault_bitmap() const
{
    std::uint32_t bitmap = 0;
    for (std::uint8_t i = 0; i < fault_count_; ++i) {
        if (faults_[i].active) {
            bitmap |= static_cast<std::uint32_t>(faults_[i].code);
        }
    }
    return bitmap;
}

}  // namespace ace::safety
