#pragma once

// =============================================================================
// ACE Device Configuration
// =============================================================================
// Bu dosya derleme zamanında sabit olan donanım kimliği ve fiziksel sistem
// sınırlarını tanımlar.
//
// NASIL DEĞİŞTİRİLİR:
//   Bu değerleri değiştirmek için firmware'in yeniden derlenmesi gerekir.
//   Runtime'da güncellenebilecek parametreler için PersistentConfig kullanın.
//
// KURAL:
//   Bu dosyada sadece constexpr sabitler bulunur; malloc, RTOS veya donanım
//   bağımlılıkları yoktur.
// =============================================================================

#include <cstdint>

namespace ace::config::device {

// -----------------------------------------------------------------------------
// Device Identity
// -----------------------------------------------------------------------------

static constexpr const char* kDeviceName       = "ACE-PT-01";
static constexpr std::uint16_t kDeviceId       = 0xACE1;
static constexpr const char* kFirmwareVersion  = "1.0.0";
static constexpr const char* kProtocolVersion  = "0.1.0";
static constexpr const char* kBuildTarget      = "Generic";  // Örn: "ESP32-S3"

// -----------------------------------------------------------------------------
// Hardware Angle Limits (Mechanical / Physical)
// Montaj kaynaklı mekanik sınırlar. Bu değerleri aşmak donanıma zarar verir.
// Yazılımsal limitler (PersistentConfig) bu değerleri asla aşamaz.
// -----------------------------------------------------------------------------

static constexpr float kPanHardMinDeg  = -180.0f;
static constexpr float kPanHardMaxDeg  =  180.0f;

static constexpr float kTiltHardMinDeg = -90.0f;
static constexpr float kTiltHardMaxDeg =  90.0f;

// -----------------------------------------------------------------------------
// Hardware Velocity Limits (Mechanical / Driver)
// Motorun ve sürücünün kaldırabileceği maksimum hız değerleri.
// -----------------------------------------------------------------------------

static constexpr float kPanMaxVelocityDps  = 120.0f;  // Motor mekanik üst limiti
static constexpr float kTiltMaxVelocityDps = 90.0f;

// -----------------------------------------------------------------------------
// Hardware Acceleration Limits
// -----------------------------------------------------------------------------

static constexpr float kPanMaxAccelDps2  = 200.0f;
static constexpr float kTiltMaxAccelDps2 = 150.0f;

// -----------------------------------------------------------------------------
// Geographic / Target Limits
// Cihazın varlık sınırlarını belirleyen coğrafi değerler.
// -----------------------------------------------------------------------------

static constexpr double kLongitudeMin = -180.0;
static constexpr double kLongitudeMax =  180.0;
static constexpr double kLatitudeMin  =  -90.0;
static constexpr double kLatitudeMax  =   90.0;
static constexpr double kAltitudeMin  = -500.0;
static constexpr double kAltitudeMax  = 9000.0;

// -----------------------------------------------------------------------------
// Control Loop Timing
// -----------------------------------------------------------------------------

static constexpr float kControlLoopPeriodS = 0.001f;  // 1 ms / 1000 Hz
static constexpr float kTelemetryPeriodS   = 0.020f;  // 20 ms / 50 Hz

// -----------------------------------------------------------------------------
// Telemetry Features String
// GET_INFO cevabında FEATURES alanı olarak yayınlanır.
// -----------------------------------------------------------------------------

static constexpr const char* kFeatures = "PAN,TILT,LOCATION,TARGET,TELEMETRY,PID,MOTION_LIMITS";

// -----------------------------------------------------------------------------
// Serial Number Default
// Her cihaza sahada SET_SERIAL komutu ile benzersiz bir seri no yazilir.
// NVS bos ise bu varsayilan deger kullanilir.
// -----------------------------------------------------------------------------

static constexpr const char* kDefaultSerialNumber = "SN-0000";

// -----------------------------------------------------------------------------
// Hardware Feature Flags
// =============================================================================
// Bu flaglar donanım yapılandırmasını derleme zamanında belirler.
// Bir donanım bileşeni eklendiğinde sadece ilgili flag'i true yapın ve
// firmware'i yeniden derleyin. AxisManager otomatik doğru yolu seçer.
//
// KULLANIM:
//   kHasImu         = true   → IMU üzerinden Compass + Accel + Gyro okunur.
//                              Tilt için ivmeölçer, Pan için pusula kullanılır.
//
//   kHasEncoder     = true   → Optik/Manyetik encoder verisi doğrudan
//                              current_position_deg olarak kullanılır.
//                              ComplementaryFilter bypass edilir.
//
//   kHasLimitSwitch = true   → HOME/CALIBRATE komutunda motor yavaşça limit
//                              switch'e kadar çevrilir, orada sıfırlanır.
//                              false iken IMU pusula seed'iyle anında sıfırlanır.
// -----------------------------------------------------------------------------

static constexpr bool kHasImu          = true;   // Minimum donanım — her zaman true
static constexpr bool kHasEncoder      = false;  // Optik/Manyetik enkoder var mı?
static constexpr bool kHasLimitSwitch  = false;  // Limit switch var mı?

// Complementary Filter hassasiyeti (yalnızca kHasEncoder=false durumunda aktif)
// Yüksek alpha → Gyro/Step'e yüksek güven, düşük drift düzeltme
// Düşük alpha  → Pusula/Accel'e daha fazla güven, daha yavaş tepki
static constexpr float kFusionAlpha = 0.98f;

}  // namespace ace::config::device
