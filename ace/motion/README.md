# ace/motion — Hareket Planlayıcı (MotionPlanner)

Bir eksenin anlık motor komutuna direkt olarak hedef açıyı yüklemek, mekanik sarsıntıya ve PID aşımına (overshoot) yol açar. `MotionPlanner` bu sorunu çözer: Hedefe **ivmeli (Trapezoidal Velocity Profile)** ile yönelir.

---

## Veri Akışı

```
AxisManager → set_target(hedef_deg)  [Komut geldiğinde bir kere]
                      │
                      ▼
             MotionPlanner (iç plan üretildi)

Scheduler → update(dt)  [Her 1ms tick'te]
                      │
                      ▼
              MotionState { position_deg, velocity_dps }
                      │
                      ▼
         Controller::update(setpoint=position_deg, measurement, dt)
```

---

## Input / Output Tablosu

| Input | Tip | Kaynak | Açıklama |
|---|---|---|---|
| `target_deg` | `float` | `AxisManager::set_angle()` | Hedefe varılacak açı |
| `max_velocity_dps` | `float` | `PersistentConfig` | Maksimum sürüş hızı |
| `max_acceleration_dps2` | `float` | `PersistentConfig` | Maksimum ivmelenme |
| `dt` | `float` | Scheduler | Her tick'teki geçen süre |

| Output | Tip | Hedef | Açıklama |
|---|---|---|---|
| `MotionState.position_deg` | `float` | `Controller` setpoint | O tick'teki ideal konum |
| `MotionState.velocity_dps` | `float` | `AxisStatus` (telemetri) | O tick'teki anlık hız |

---

## Hareket Profili (Trapezoidal)

```
Hız
 │           ┌──────────┐
 │          /│          │\
 │         / │          │ \
 │        /  │          │  \
 └───────────────────────────── Zaman
     İvme     Sabit Hız    Fren
```

1. **İvme Fazı:** Motor `max_velocity_dps`'e ulaşana dek her tick'te hız artırılır.
2. **Sabit Hız Fazı:** Durma mesafesi kontrolüne kadar sabit kalır.
3. **Fren Fazı:** Hedefe yeterince yaklaşıldığında yavaşlama başlar.

---

## Bağımlılıklar

Bu modülün diğer `ace/*` modüllerine bağımlılığı **yoktur**. Sadece `<cmath>` kullanır.
