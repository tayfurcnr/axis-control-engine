# ACE / Config Module

Bu modül, sistemin tüm konfigürasyon katmanını yönetir.

İki ayrı konfigürasyon katmanına ayrılmıştır:

| Katman | Dosya | Ne Zaman Değişir? |
|---|---|---|
| **Donanım Profili** | `device_config.hpp` | Sadece derleme zamanında |
| **Çalışma Parametreleri** | `config.hpp` + NVS | Komutla, runtime'da |

---

## 1. `device_config.hpp` — Donanım Profili

Cihazın fiziksel kimliğini ve mekanik sınırlarını tanımlar. Tüm değerler `constexpr`'dir; değiştirmek için firmware yeniden derlenir.

### Cihaz Kimliği Nasıl Değiştirilir?

```cpp
// ace/config/device_config.hpp

static constexpr const char*     kDeviceName      = "ACE-PT-01";  // ← değiştirin
static constexpr std::uint16_t   kDeviceId        = 0xACE1;       // ← değiştirin
static constexpr const char*     kFirmwareVersion = "1.0.0";      // ← arttırın
static constexpr const char*     kBuildTarget     = "Generic";    // Örn: "ESP32-S3"
```

Değişiklik sonrası `GET_INFO` komutuna verilen cevap otomatik güncellenir.

### Fiziksel (Mekanik) Limitler Nasıl Değiştirilir?

```cpp
// ace/config/device_config.hpp

// Montaj mekanik açı aralığı:
static constexpr float kPanHardMinDeg  = -180.0f;   // ← montajınıza göre ayarlayın
static constexpr float kPanHardMaxDeg  =  180.0f;
static constexpr float kTiltHardMinDeg =  -90.0f;
static constexpr float kTiltHardMaxDeg =   90.0f;

// Sürücünün kaldırabileceği maksimum hız:
static constexpr float kPanMaxVelocityDps  = 120.0f;
static constexpr float kTiltMaxVelocityDps =  90.0f;
```

> **Önemli:** Bu değerler **donanım fiziğini** temsil eder. Hiçbir operatör komutu bu sınırları aşamaz. Sistem çift kontrol uygular: önce hard limit, sonra soft limit.

---

## 2. `config.hpp` (`PersistentConfig`) — Çalışma Parametreleri

Operatörün çalışma sırasında değiştirebildiği parametrelerdir. NVS (Non-Volatile Storage) üzerinde saklanır; cihaz resetlense bile korunur.

Fabrika varsayılan değerleri `device_config.hpp`'ten beslenir.

### Hangi Parametreler Komutla Değiştirilebilir?

| Alan | Komut | Açıklama |
|---|---|---|
| `pan_pid`, `tilt_pid` | `SET_PID <AXIS> <KP> <KI> <KD> <FF> <MAX_INT> <MAX_OUT>` | PID kazançları |
| `pan_motion`, `tilt_motion` | `SET_MOTION_LIMITS <AXIS> <MAX_VEL> <MAX_ACCEL>` | Hareket hız/ivme sınırı |
| `location_*` | `SET_LOCATION <LON> <LAT> <ALT>` | Cihazın coğrafi konumu |

### Soft Limit ile Hard Limit Farkı

```
Operatör SET_ANGLE PAN 200 gönderir
        │
        ▼
Hard Limit (device_config) → kPanHardMaxDeg = 180 → ❌ NACK (donanım sınırı)

Operatör SET_ANGLE PAN 150 gönderir
        │
        ▼
Hard Limit → 150 < 180 → ✅
        │
        ▼
Soft Limit (PersistentConfig.pan_angle_limit) → örn: max=120 → ❌ NACK (yazılım sınırı)

Operatör SET_ANGLE PAN 100 gönderir → ✅ Her iki sınırı da geçti → Komut işlenir
```

---

## 3. Persistent Storage

Konfigürasyon NVS'e `blob` olarak kaydedilir.

### Sürüm Uyumsuzluğu Koruması

`PersistentConfig` yapısına yeni bir alan eklendiğinde veya `device_config.hpp`'de büyük bir değişiklik yapıldığında `kVersion` sayısı artırılmalıdır:

```cpp
// ace/config/config.hpp

struct PersistentConfig {
    static constexpr std::uint32_t kVersion = 3;  // ← buraya +1 ekleyin
    ...
};
```

Cihaz açıldığında NVS'deki sürüm (`loaded_version`) ile derleme zamanı sürümü (`kVersion`) karşılaştırılır. Uyuşmazsa NVS verisi reddedilir ve `device_config.hpp` fabrika değerleri yüklenir.

```
NVS version=3, kVersion=4 → uyumsuz → fabrika değerleri yüklenir ✅
NVS version=4, kVersion=4 → uyumlu  → NVS değerleri yüklenir   ✅
NVS bulunamadı            → ilk açılış → fabrika değerleri yüklenir ✅
```

### Fabrika Ayarlarına Dönmek

`FACTORY_RESET` komutu gönderildiğinde NVS silinir ve `device_config.hpp`'deki varsayılan değerler yeniden devreye girer.

---

## 4. Dosya Listesi

| Dosya | Açıklama |
|---|---|
| `device_config.hpp` | Derleme zamanı donanım profili (constexpr) |
| `config.hpp` | Runtime konfigürasyon yapıları (`PersistentConfig` vb.) |
| `persistent_storage.hpp` | NVS arayüz tanımı (interface) |
| `persistent_storage.cpp` | ESP32 NVS implementasyonu |
| `persistent_storage_interface.hpp` | Platform bağımsız soyut arayüz (test mockları için) |
