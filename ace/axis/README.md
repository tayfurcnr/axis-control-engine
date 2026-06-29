# ace/axis — Eksen Yöneticisi (AxisManager)

Sistemin sinir merkezi. Gelen komutları işler, durum makinesini (state machine) yönetir, hareket planlayıcıyı ve PID döngüsünü her tick'te çalıştırır, olayları (event) üretir.

---

## Veri Akışı

```
[Protocol / GCS]
      │  CommandRequest
      ▼
 AxisManager::execute()
      │
      ├─→ Limit Kontrolü (device_config hard -> PersistentConfig soft)
      ├─→ Durum Geçişi (boot → ready → position/velocity/tracking ...)
      └─→ ACK / NACK / EventMessage

[Scheduler — 1 ms tick]
      │  SensorData (Pusula, Accel, Gyro, Step Hızı)
      ▼
 AxisManager::update(dt, sensors)
      │
      ├─→ ComplementaryFilter → current_position_deg (Pan + Tilt)
      ├─→ MotionPlanner::update(dt) → anlık setpoint
      ├─→ Controller::update(setpoint, measurement) → effort (% PWM)
      └─→ AxisStatus güncellenir → Telemetry'e hazır
```

---

## Input / Output Tablosu

| Kaynak | Veri | Fonksiyon |
|---|---|---|
| Protocol/GCS | `CommandRequest` | `execute()` |
| Scheduler (1ms) | `SensorData` (pan_rate_dps, tilt_rate_dps, compass_heading_deg, accel_tilt_deg) | `update(dt, sensors)` |
| Config | `PersistentConfig` (PID kazançları, limit) | `bind_persistent_state()` |

| Hedef | Veri | Fonksiyon |
|---|---|---|
| Protocol/GCS | `CommandOutcome` (ACK / NACK) | `execute()` return |
| Telemetry | `AxisStatus` (pan + tilt) | `pan_status()` / `tilt_status()` |
| Sistem | `EventMessage` (MOTION_DONE vb.) | `consume_event()` |
| HAL/Drivers | `effort` (% PWM) | `Controller::update()` çıktısı *(HAL entegrasyonunda kullanılacak)* |

---

## Durum Makinası

```
boot ──enable──→ ready ──SET_ANGLE──→ position ──hedefe ulaşıldı──→ ready
                   │                                                   │
                   ├──SET_VELOCITY──→ velocity ──STOP──────────────────┘
                   ├──HOME──────────→ homing
                   ├──CALIBRATE─────→ calibration
                   └──FAULT tetik───→ fault
```

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/communication` | `CommandRequest`, `CommandId`, `AxisId` tipleri |
| `ace/config` | `PersistentConfig` (runtime parametreler), `device_config.hpp` (hard limitler) |
| `ace/control` | `Controller` (PID), `ComplementaryFilter` (sensör füzyonu) |
| `ace/motion` | `MotionPlanner` (ivmeli hareket profili) |
| `ace/services` | `Logger` (debug çıktısı) |

---

## Önemli Notlar

- `execute()` / `update()` **thread-safe değildir**; Scheduler aynı çekirdeğe pinlenmeli.
- `SensorData.pan_rate_dps` → Açık döngü sürüşte step motor komut hızından, HAL entegre edilince Gyro'dan gelir.
- `FACTORY_RESET` seri numarasını silmez, yalnızca `PersistentConfig` parametrelerini sıfırlar.
