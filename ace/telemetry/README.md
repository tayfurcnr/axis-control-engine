# ace/telemetry — Telemetri Yayıncısı (TelemetryPublisher)

Sistemin anlık durumunu (konum, mod, hata, hız) periyodik olarak GCS'e ileten veri yayın modülü. Sistemi izlemek ve kayıt altına almak için kullanılır.

---

## Veri Akışı

```
AxisManager → pan_status() / tilt_status()
SafetyManager → fault flags
                  │
                  ▼
       TelemetryPublisher::publish()   [~50 Hz / 20ms]
                  │
                  ▼
       "TELEMETRY ENABLED=1 MOVING=0 PAN_ANGLE=45.1 TILT_ANGLE=5.0 MODE=TRACK ..."
                  │
                  ▼
         UART / UDP → GCS ekranı
```

---

## Input / Output Tablosu

| Input | Tip | Kaynak | Açıklama |
|---|---|---|---|
| `AxisStatus` (pan + tilt) | Struct | `AxisManager` | Anlık konum, hız, durum |
| `ModeId` | Enum | `AxisManager` | MANUAL / TRACK / AUTO |
| Fault Flag | `bool` | Safety / Diagnostics | Hata durumu |

| Output | Format | Hedef |
|---|---|---|
| Telemetri dizesi | `PROTOCOL_REFERENCE` ALP formatı | UART / UDP (GCS) |

---

## Telemetri Çıktısı Örneği

```
TELEMETRY ENABLED=1 MOVING=0 MODE=TRACK
PAN_ANGLE=45.12 PAN_VEL=0.00
TILT_ANGLE=5.05 TILT_VEL=0.00
FAULT=0
```

---

## Yayın Frekansı

| Görev | Frekans |
|---|---|
| Telemetri yayını | 50 Hz (20 ms) |
| Hata log yayını | Event tetiklemeli (asenkron) |

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/axis` | `AxisStatus` (konum/hız/durum verileri) |
| `ace/communication` | `TelemetryResponse` veri modeli |
