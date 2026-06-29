# ace/communication — Haberleşme Tipleri ve Modeller

Sistemin ortak veri dili. Hiçbir iş mantığı içermez; sadece tüm modüllerin birbirine veri iletmek için kullandığı `enum`, `struct` ve `id` tanımlamalarını barındırır.

---

## İçindekiler

### `command_ids.hpp` — Komut Kimlikleri

Protokolden gelen her komutun sabit sayısal kimliği.

| ID (Hex) | İsim | Açıklama |
|---|---|---|
| `0x10` | `ENABLE` / `DISABLE` | Eksen aktifleştirme |
| `0x20` | `SET_ANGLE` | Konuma git |
| `0x21` | `SET_VELOCITY` | Hızla git |
| `0x22` | `STOP` | Dur |
| `0x30` | `SET_PID` | PID kazanç güncelleme |
| `0x31` | `SET_MOTION_LIMITS` | Hız/ivme limiti |
| `0x40` | `HOME` / `CALIBRATE` | Sıfırlama / Kalibrasyon |
| `0x50` | `HEARTBEAT` | Bağlantı testi |
| `0x60` | `SET_LOCATION` | Cihaz GPS konumu |
| `0x61` | `SET_DEBUG` | Debug çıktısı aç/kapat |
| `0x62` | `SET_SERIAL` | Seri numara ata |
| `0x80` | `GET_INFO` | Cihaz bilgisi iste |
| `0x90` | `REBOOT` | Yeniden başlat |
| `0xA0` | `FACTORY_RESET` | Fabrika ayarları |

---

### `message_models.hpp` — Veri Taşıyıcılar

#### `CommandRequest` — GCS → AxisManager
Bir komutun tüm parametrelerini taşır.

| Alan | Tip | Hangi Komutlarda |
|---|---|---|
| `command_id` | `CommandId` | Tümü |
| `axis` | `AxisId` | SET_ANGLE, SET_PID... |
| `target_angle_pan_deg` | `float` | SET_ANGLE |
| `target_angle_tilt_deg` | `float` | SET_ANGLE |
| `target_velocity_dps` | `float` | SET_VELOCITY |
| `kp / ki / kd / ff` | `float` | SET_PID |
| `max_velocity_dps` | `float` | SET_MOTION_LIMITS |
| `max_acceleration_dps2` | `float` | SET_MOTION_LIMITS |
| `longitude/latitude/altitude` | `double` | SET_LOCATION, SET_TARGET |
| `serial_number[32]` | `char[]` | SET_SERIAL |

#### `CommandOutcome` — AxisManager → GCS
Komutun sonucu (başarı/hata).

```
CommandOutcome
 ├── ack: AckResponse { command_id }   // Başarı
 └── nack: NackResponse { command_id, error_code }  // Hata
```

#### `EventMessage` — AxisManager → GCS (Asenkron)
Uzun süren bir hareket tamamlandığında otomatik üretilir.

```
EventMessage { event_type: MOTION_DONE, source_command_id }
```

#### `TelemetryResponse` — AxisManager → Telemetry → GCS
Anlık durum verisi.

---

## Veri Akışı

```
GCS (UART/Network)
    │ raw string: "SET_ANGLE PAN 45"
    ▼
Protocol (AlpParser)
    │ CommandRequest { SET_ANGLE, PAN, 45.0 }
    ▼
AxisManager::execute()
    │ CommandOutcome { ACK }
    ▼
Protocol → GCS
```

---

## Bağımlılıklar

Bu modülün diğer `ace/*` modüllerine bağımlılığı **yoktur**. Saf veri tipleri.
