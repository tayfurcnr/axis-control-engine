# ace/diagnostics — Sistem Sağlık İzleyici (Diagnostics)

Cihazın donanım sağlığını periyodik olarak ölçer ve kritik eşikler aşıldığında `SafetyManager`'ı uyarır.

---

## Veri Akışı

```
HAL / Drivers
    │  ADC (Voltaj, Akım) / Sıcaklık sensörü / GPIO (Limit Switch)
    ▼
Diagnostics::check()   [10 Hz / 100ms]
    │
    ├─→ Tüm metrikler sağlıklı   →  Normal çalışmaya devam
    └─→ Eşik aşıldı              →  SafetyManager::report_fault(FaultCode)
                                          │
                                          ▼
                                   AxisManager → state = fault
                                   Telemetri → FAULT=1
```

---

## İzlenen Metrikler

| Metrik | Kaynak | Kritik Eşik |
|---|---|---|
| Motor sürücü gerilimi (V) | ADC | < 10V veya > 30V |
| Motor akımı (A) | Akım sensörü | > max_current_A (config) |
| Motor/Sürücü sıcaklığı (°C) | NTC / DS18B20 | > 70°C |
| MCU sıcaklığı (°C) | ESP32 iç sensör | > 85°C |
| IMU bağlantısı | I2C ACK | Cevap yok |
| Limit switch durumu | GPIO | Tetiklendi (isteğe bağlı) |

---

## Input / Output Tablosu

| Input | Tip | Kaynak |
|---|---|---|
| ADC ölçümleri | `float` | `IMotorDriver` / ADC HAL |
| IMU erişilebilirlik | `bool` | `IImu::is_alive()` |
| Limit switch | `bool` | GPIO HAL |

| Output | Tip | Hedef |
|---|---|---|
| Hata kodu | `FaultCode` enum | `SafetyManager::report_fault()` |
| Sağlık raporu | `DiagnosticsReport` struct | `TelemetryPublisher` / Logger |

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/hal` | Sensör okuma arayüzleri |
| `ace/safety` | Hata iletilmesi |
| `ace/services` | Logger (hata kayıtları) |
