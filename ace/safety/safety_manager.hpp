#pragma once

#include <cstdint>
#include <optional>

namespace ace::safety {

// =============================================================================
// Hata Kodları (Fault Codes)
// =============================================================================
// Her bit veya enum değeri bağımsız bir hata kaynağını temsil eder.
// Böylece birden fazla hata aynı anda aktif olabilir.
// =============================================================================
enum class FaultCode : std::uint32_t {
    NONE                   = 0x00000000,

    // --- Yazılım / Durum Hataları ---
    AXIS_PAN_OVERRUN       = 0x00000001,  // Pan hard limit aşıldı
    AXIS_TILT_OVERRUN      = 0x00000002,  // Tilt hard limit aşıldı
    COMMAND_TIMEOUT        = 0x00000004,  // GCS bağlantısı zaman aşımına uğradı
    INVALID_STATE          = 0x00000008,  // İzin verilmeyen durum geçişi

    // --- Donanım Hataları (HAL bağlanınca tetiklenecek) ---
    OVER_TEMPERATURE       = 0x00000010,  // Motor/Sürücü sıcaklığı aşıldı
    UNDER_VOLTAGE          = 0x00000020,  // Besleme gerilimi düşük
    OVER_CURRENT           = 0x00000040,  // Motor akımı aşıldı
    IMU_UNAVAILABLE        = 0x00000080,  // IMU bağlantısı yok / yanıt vermiyor
    ENCODER_FAULT          = 0x00000100,  // Encoder sinyal kaybı

    // --- Kritik / Acil ---
    EMERGENCY_STOP         = 0x80000000,  // Operator tarafından tetiklendi
};

// Hata öncelik seviyesi
enum class FaultSeverity : std::uint8_t {
    WARNING   = 0,  // Uyarı — çalışmaya devam edilir, GCS'e bildirilir
    CRITICAL  = 1,  // Kritik — eksenler durdurulur, güvenli beklemeye geçilir
    EMERGENCY = 2,  // Acil — anında motorlar kesilir, sadece reboot ile çıkılır
};

// Aktif bir hatanın snapshot'ı
struct FaultEntry {
    FaultCode     code     = FaultCode::NONE;
    FaultSeverity severity = FaultSeverity::WARNING;
    bool          active   = false;
};

// =============================================================================
// SafetyManager
// =============================================================================
class SafetyManager {
public:
    static constexpr std::uint8_t kMaxFaults = 16;

    // Yeni bir hata raporla. Otomatik olarak seviyesine göre tepki verir.
    void report_fault(FaultCode code);

    // Bir hatayı temizle (örn: sıcaklık normale döndü).
    // EMERGENCY_STOP manuel clear edilemez — sadece reboot ile temizlenir.
    bool clear_fault(FaultCode code);

    // Tüm non-emergency hataları temizle (örn: FACTORY_RESET sonrası)
    void clear_all();

    // Sistemi anında güvenli beklemeye al (EMERGENCY_STOP veya harici çağrı).
    void enter_safe_state();

    // Güvenli bekleme kilidi açılabilir mi?
    // Sadece hiç CRITICAL/EMERGENCY hata kalmadıysa true döner.
    [[nodiscard]] bool can_exit_safe_state() const;

    // Safe state aktif mi?
    [[nodiscard]] bool is_safe_state_active() const;

    // Aktif herhangi bir hata var mı?
    [[nodiscard]] bool has_fault() const;

    // Belirli bir hata aktif mi?
    [[nodiscard]] bool has_fault(FaultCode code) const;

    // En yüksek öncelikli aktif hatayı döndür
    [[nodiscard]] std::optional<FaultEntry> highest_active_fault() const;

    // Tüm aktif hata kodlarını bitmap olarak döndür (telemetri için)
    [[nodiscard]] std::uint32_t active_fault_bitmap() const;

private:
    static FaultSeverity severity_of(FaultCode code);
    bool safe_state_active_ = false;
    FaultEntry faults_[kMaxFaults] = {};
    std::uint8_t fault_count_ = 0;
};

}  // namespace ace::safety
