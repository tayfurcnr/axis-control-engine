# ace/hal — Donanım Soyutlama Katmanı (HAL)

Yazılım mantığı (AxisManager, Controller) ile platforma özgü donanım kodu (Drivers) arasındaki köprü. Tüm donanım bağlantıları saf `virtual` C++ arayüzleri (interface) olarak buraya tanımlanır.

---

## Neden HAL?

```
Kötü Tasarım:
  AxisManager → esp32_mcpwm_set_duty(...)   ← ESP32'ye kilitli, test edilemez

İyi Tasarım (HAL):
  AxisManager → IMotorDriver::set_effort()  ← Soyut, test edilebilir

  Platform A (ESP32):    class Esp32MotorDriver  : public IMotorDriver { ... }
  Platform B (Test):     class MockMotorDriver   : public IMotorDriver { ... }
```

---

## Arayüzler (Interfaces)

### `IMotorDriver` — Motor Sürücü Arayüzü

| Metod | Input | Output | Açıklama |
|---|---|---|---|
| `set_effort(float)` | [0.0–1.0] tork oranı | — | PWM duty cycle'ı ayarlar |
| `enable()` | — | — | Motoru aktifleştirir |
| `disable()` | — | — | Motoru serbest bırakır (coast) |

### `IEncoder` — Enkoder Okuyucu Arayüzü
*(Gelecek sürümde—şu an IMU + Step Counting kullanılıyor)*

| Metod | Input | Output | Açıklama |
|---|---|---|---|
| `read_angle_deg()` | — | `float` | Anlık mekanik açı |
| `reset()` | — | — | Sıfırlama |

### `IImu` — IMU Arayüzü

| Metod | Output | Açıklama |
|---|---|---|
| `read_compass_heading_deg()` | `float` | Kuzey'e göre Pan açısı |
| `read_accel_tilt_deg()` | `float` | Yerçekimine göre Tilt açısı |
| `read_gyro_dps()` | `float[3]` | Açısal hız (dps) |

---

## Veri Akışı

```
[Scheduler — 1ms]
        │
        ▼
IImu::read_*(...)  →  SensorData  →  AxisManager::update(dt, sensors)
                                              │
                                              ▼
                                     Controller → effort
                                              │
                                              ▼
                                    IMotorDriver::set_effort(effort)
                                              │
                                              ▼
                                     [Drivers: mcpwm / step pulse]
```

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/axis` | `SensorData` ve `AxisManager`'a veri besler |
| `ace/drivers` | HAL arayüzlerinin gerçek ESP32 implementasyonları burada |
