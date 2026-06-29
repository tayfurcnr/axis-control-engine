# ace/drivers — Donanım Sürücüleri (ESP32-S3)

Doğrudan ESP32-S3 donanım registerlarıyla konuşan en alt katman. HAL arayüzlerini `override` ederek somut gerçekleştirimlerini sağlar. **Sadece ESP32 hedefinde derlenir.**

---

## Bileşenler

### Step Motor Sürücüsü (Step/Dir)

Step motorları sürmek için `ESP32 MCPWM` veya `GPIO` periyodik pulse üretimi kullanılır.

#### Veri Akışı

```
Controller → IMotorDriver::set_effort(effort)
                    │
                    ▼
         Esp32StepDriver::set_effort()
                    │
                    ├─→ effort → pulse frekansına dönüştürülür
                    │           (Örn: 0.5 effort → step/s hız)
                    └─→ gpio_set_level(DIR_PIN, direction)
                        mcpwm_timer_start / ledc_set_duty(STEP_PIN, freq)
```

#### Input / Output

| Input | Tip | Kaynak |
|---|---|---|
| `effort` | `float` [0.0–1.0] | `Controller::update()` |
| `direction` | `bool` | Motor yönü (+/-) |

| Output | Sinyal | Pin |
|---|---|---|
| Step Pulse | PWM | STEP_PIN (Config'de tanımlı) |
| Yön | GPIO HIGH/LOW | DIR_PIN (Config'de tanımlı) |

---

### IMU Sürücüsü (I2C / SPI)

9-DOF IMU'dan (Örn: MPU9250, BNO055, ICM-42688) ham sensör verisi okunur.

#### Veri Akışı

```
Scheduler (1ms tick)
        │
        ▼
Esp32ImuDriver::read_all()
        │  I2C / SPI register okuma
        ▼
SensorData { pan_rate_dps, accel_tilt_deg, compass_heading_deg }
        │
        ▼
AxisManager::update(dt, sensors)
```

#### Input / Output

| Input | Fiziksel Kaynak |
|---|---|
| — | IMU chip (I2C/SPI bus) |

| Output | Tip | Hedef |
|---|---|---|
| `SensorData` | Struct | `AxisManager::update()` |

---

## Platform Bağımlılığı

| Alan | Değer |
|---|---|
| Hedef Çip | ESP32-S3 |
| SDK | ESP-IDF v5.x |
| Kullanılan Periferaller | `mcpwm`, `ledc`, `i2c_master`, `gpio` |
| PC'de Derlenir mi? | **Hayır** — CMake `target_platform` koşuluna bağlıdır |

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/hal` | `IMotorDriver`, `IImu` arayüzlerini gerçekleştirir |
| `ace/config` | Pin numaraları ve donanım profili `device_config.hpp`'ten alınır |
