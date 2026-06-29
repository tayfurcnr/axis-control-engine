# ace/safety — Güvenlik Yöneticisi (SafetyManager)

Kritik arıza anında veya `EMERGENCY_STOP` komutunda sistemi **Güvenli Bekleme (Safe State)** moduna sokan emniyet katmanı. Tüm diğer yazılım mantığının üzerinde önceliğe sahiptir.

---

## Veri Akışı

```
Diagnostics → Sınır aşımı (sıcaklık, gerilim, akım)
AxisManager → Fault flag
GCS         → EMERGENCY_STOP komutu
                    │
                    ▼
         SafetyManager::evaluate()
                    │
                    ├─→ FAULT yok    → Normal çalışmaya devam
                    └─→ FAULT var    → safe_state_enter()
                                           │
                                           ├─→ IMotorDriver::disable()  (Motorlar boşa alınır)
                                           ├─→ AxisManager → state = FAULT
                                           └─→ Telemetry → FAULT=1 yayını başlar
```

---

## Input / Output Tablosu

| Input | Tip | Kaynak |
|---|---|---|
| Diagnostics alarm | `bool` flags | `Diagnostics::check()` |
| Axis fault flag | `bool` | `AxisManager` |
| `EMERGENCY_STOP` komutu | `CommandRequest` | Protocol / GCS |

| Output | Tip | Hedef |
|---|---|---|
| `safe_state` sinyali | IMotorDriver::disable() | Drivers / HAL |
| FAULT state | `AxisState::fault` | AxisManager |
| Telemetri alarmı | `FAULT=1` | GCS / Operator |

---

## Safe State Davranışı

| Durum | Motor | Fren |
|---|---|---|
| `EMERGENCY_STOP` | Anında kesil (`disable`) | Sert fren |
| Sıcaklık aşımı | Yavaşça dur → disable | Yumuşak fren |
| Haberleşme koptu (Timeout) | Dur → ready bekleme | Yumuşak fren |

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/hal` | `IMotorDriver::disable()` çağrısı |
| `ace/axis` | Durum güncellemesi (`state = fault`) |
| `ace/diagnostics` | Sağlık metrikleri alarmları |
