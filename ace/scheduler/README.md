# ace/scheduler — Görev Zamanlayıcı (Scheduler / FreeRTOS)

Tüm modüllerin periyodik çalıştırılmasından sorumlu koordinatör. FreeRTOS task'larını ve timer'larını oluşturur; CPU yükünü adaletli dağıtır.

---

## Görev Mimarisi

```
FreeRTOS Tasks (ESP32-S3)

┌────────────────────────────────────────────────────┐
│  Task: ControlLoop          [Çekirdek 1]            │
│  Frekans: 1000 Hz (1ms)                            │
│  ─────────────────────────────────────────────────  │
│  IMU::read() → SensorData                          │
│  AxisManager::update(dt, sensors)                  │
│  IMotorDriver::set_effort(effort)                  │
└────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────┐
│  Task: ProtocolRx           [Çekirdek 0]            │
│  Frekans: UART RX event tetiklemeli (asenkron)     │
│  ─────────────────────────────────────────────────  │
│  AlpParser::parse(line) → CommandRequest           │
│  Queue → ControlLoop task'a ilet                   │
└────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────┐
│  Task: TelemetryTx          [Çekirdek 0]            │
│  Frekans: 50 Hz (20ms)                             │
│  ─────────────────────────────────────────────────  │
│  TelemetryPublisher::publish()                     │
│  UART TX → GCS                                     │
└────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────┐
│  Task: DiagnosticsLoop      [Çekirdek 0]            │
│  Frekans: 10 Hz (100ms)                            │
│  ─────────────────────────────────────────────────  │
│  Diagnostics::check()                              │
│  SafetyManager::evaluate()                         │
└────────────────────────────────────────────────────┘
```

---

## Inter-Task Haberleşme

| Kanal | Tip | Kimden | Kime |
|---|---|---|---|
| Komut kuyruğu | FreeRTOS Queue | ProtocolRx | ControlLoop |
| Telemetri verisi | FreeRTOS Queue | ControlLoop | TelemetryTx |
| Safety sinyal | FreeRTOS Event Group | DiagnosticsLoop | ControlLoop |

---

## Frekans Tablosu

| Görev | Frekans | Periyot |
|---|---|---|
| ControlLoop (PID + Sensör) | 1000 Hz | 1 ms |
| Telemetry yayını | 50 Hz | 20 ms |
| Diagnostics sağlık kontrolü | 10 Hz | 100 ms |
| Protocol Rx | Asenkron (UART event) | — |

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/axis` | ControlLoop'ta `update()` çağrılır |
| `ace/telemetry` | TelemetryTx task'ında `publish()` çağrılır |
| `ace/protocol` | ProtocolRx task'ında parse edilir |
| `ace/diagnostics` | DiagnosticsLoop task'ında `check()` çağrılır |
| `ace/safety` | DiagnosticsLoop → SafetyManager::evaluate() |
