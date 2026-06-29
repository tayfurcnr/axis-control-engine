# Axis Control Engine (ACE)

**ACE**, pan-tilt ekseni kontrol sistemleri için geliştirilmiş, platform bağımsız, gömülü C++ altyapısıdır. Hedef platform ESP32-S3 olmakla birlikte, kontrol algoritmaları ve mantık katmanı donanımdan tamamen yalıtılmıştır.

---

## Özellikler

- **PID Kapalı Döngü Kontrol** — İvmeölçer (Accel) ve Pusula (Compass) füzyonuyla desteklenmiş
- **Complementary Filter (Sensör Füzyonu)** — IMU temelli konum tahmini (Gyro + Compass/Accel)
- **Trapezoidal Hareket Planlayıcı** — Ani sarsıntısız ivmeli hareket profili
- **Kalıcı Konfigürasyon (NVS)** — PID kazançları, hareket limitleri, seri numara
- **Çift Limitli Güvenlik** — Hard limit (derleme zamanı) + Soft limit (runtime, komutla değiştirilebilir)
- **ALP Protokolü** — Text tabanlı insan okunır UART/UDP komut arayüzü
- **Seri Numara Yönetimi** — Factory Reset sonrasında dahi korunan cihaz kimliği
- **PC'de Test Edilebilir** — Donanım gerektirmeden `g++` ile birim testleri

---

## Modül Haritası

| Modül | Dizin | Satır | Açıklama |
|---|---|---|---|
| **Axis** | [`ace/axis`](ace/axis/README.md) | 78 | Eksen durum makinası, komut işleme, PID + Planner orkestrasyon |
| **Control** | [`ace/control`](ace/control/README.md) | 94 | PID algoritması + Complementary Filter (sensör füzyonu) |
| **Motion** | [`ace/motion`](ace/motion/README.md) | 62 | Trapezoidal hız profili, ivme + fren rampaları |
| **Communication** | [`ace/communication`](ace/communication/README.md) | 90 | Komut ID'leri, CommandRequest/Response veri modelleri |
| **Config** | [`ace/config`](ace/config/README.md) | 153 | Donanım profili, NVS kalıcı konfigürasyon, seri numara |
| **Protocol** | [`ace/protocol`](ace/protocol/README.md) | 80 | ALP metin protokol ayrıştırıcı ve göndericisi |
| **HAL** | [`ace/hal`](ace/hal/README.md) | 75 | Donanım soyutlama arayüzleri (IMotorDriver, IImu, IEncoder) |
| **Drivers** | [`ace/drivers`](ace/drivers/README.md) | 88 | ESP32-S3 mcpwm/I2C/GPIO sürücü implementasyonları |
| **Telemetry** | [`ace/telemetry`](ace/telemetry/README.md) | 64 | 50 Hz periyodik durum yayını (GCS'e) |
| **Safety** | [`ace/safety`](ace/safety/README.md) | 59 | Acil durdurma, Safe State emniyet yöneticisi |
| **Scheduler** | [`ace/scheduler`](ace/scheduler/README.md) | 77 | FreeRTOS task mimarisi, kontrol döngüsü zamanlama |
| **Diagnostics** | [`ace/diagnostics`](ace/diagnostics/README.md) | 59 | Voltaj/sıcaklık/IMU sağlık izleme |
| **Services** | [`ace/services`](ace/services/README.md) | 47 | Logger, debug aç/kapat |
| **Tests** | [`ace/tests`](ace/tests/README.md) | 64 | PC'de çalışan donanımsız birim testleri |

---

## Sistem Veri Akışı

```
GCS / UART
    │  "SET_ANGLE PAN 45.0\r\n"
    ▼
ace/protocol  (AlpParser)
    │  CommandRequest
    ▼
ace/axis  (AxisManager::execute)
    │  ACK / NACK → GCS
    │
    ▼  [1ms Scheduler tick]
ace/hal / ace/drivers  (IImu::read)
    │  SensorData {compass, accel, gyro}
    ▼
ace/axis  (AxisManager::update)
    ├── ace/control  (ComplementaryFilter) → current_position_deg
    ├── ace/motion   (MotionPlanner)       → setpoint_deg
    └── ace/control  (PID Controller)      → effort [0–1]
                                                │
                                                ▼
                                     ace/hal / ace/drivers
                                     (IMotorDriver::set_effort)
                                                │
                                                ▼
                                          Step Motor / BLDC

[50ms Telemetry tick]
ace/telemetry → "TELEMETRY PAN_ANGLE=45.1 MODE=TRACK ..." → GCS
```

---

## Hızlı Başlangıç (PC Test)

```bash
# AxisManager birim testleri
g++ -std=c++17 ace/tests/test_axis_manager.cpp \
    ace/axis/axis_manager.cpp ace/control/controller.cpp \
    ace/control/sensor_fusion.cpp ace/motion/motion_planner.cpp \
    ace/telemetry/telemetry_publisher.cpp ace/services/logger.cpp \
    -I. -o /tmp/test_axis && /tmp/test_axis

# Sensor Fusion testleri
g++ -std=c++17 ace/tests/test_sensor_fusion.cpp \
    ace/control/sensor_fusion.cpp \
    -I. -o /tmp/test_sf && /tmp/test_sf
```

---

## Referans Belgeler

- [PROTOCOL_REFERENCE.md](PROTOCOL_REFERENCE.md) — Tüm ALP komutları ve telemetri formatı
- [docs/ACE_PRD.md](docs/ACE_PRD.md) — Ürün Gereksinim Dokümanı
