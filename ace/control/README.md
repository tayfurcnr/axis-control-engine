# ace/control — Kontrol Algoritmaları

Sistemin matematiksel beyin katmanı. Donanımdan ve protokolden tamamen bağımsızdır. İki bileşen içerir: **PID Kontrolcüsü** ve **Complementary Filtre (Sensör Füzyonu)**.

---

## Bileşenler

### 1. `Controller` — PID Kapalı Döngü Kontrolcüsü

Her eksen için ayrı bir `Controller` örneği, `AxisManager` tarafından her 1 ms tick'te çağrılır.

#### Veri Akışı

```
MotionPlanner'dan → setpoint_deg (anlık hedef konum)
SensorFusion'dan  → measurement_deg (füzyonlanmış gerçek konum)
                    dt (geçen süre, saniye)
        │
        ▼
   Controller::update()
        │
        ▼
   effort (0.0 – 1.0 arası normalize tork/PWM çıktısı)
        │
        ▼
   HAL / Motor Sürücüsü (ileride)
```

#### Input / Output

| Input | Tip | Açıklama |
|---|---|---|
| `setpoint` | `float` (derece) | `MotionPlanner`'dan gelen anlık ideal konum |
| `measurement` | `float` (derece) | `ComplementaryFilter`'dan fuse edilmiş gerçek konum |
| `dt` | `float` (saniye) | Scheduler tick süresi (genelde 0.001) |

| Output | Tip | Açıklama |
|---|---|---|
| `effort` | `float` [0.0–1.0] | Motor sürücüye iletilecek tork/PWM yüzdesi |

#### Özellikler

- **Integral Windup Koruması:** `max_integral` sınırı aşılırsa integral birikimi kesilir.
- **Output Clamping:** `max_output` sınırı aşılırsa çıktı kırpılır.
- **Feed-Forward:** Sabit bir baz itki (`feed_forward`) sisteme eklenerek PID hatasını azaltır.
- Kazançlar (`kp`, `ki`, `kd`, `ff`, `max_integral`, `max_output`) → `PersistentConfig`'ten Hot-Reload edilir.

---

### 2. `ComplementaryFilter` — Sensör Füzyon Filtresi

IMU'nun iki farklı veri kaynağını harmanlayarak drift-free bir konum tahmini üretir.

#### Veri Akışı

```
Jiroskop / Step Motor Hızı → rate_dps  (Yüksek frekans, anlık ama zamanla kayar)
Pusula / İvmeölçer         → absolute_deg (Düşük frekans, gürültülü ama mutlak referans)
                              dt
        │
        ▼
   filter.update(rate_dps, absolute_deg, dt)
        │
        ▼
   estimate_deg → AxisManager → current_position_deg → PID'e gider
```

#### Input / Output

| Input | Tip | Kaynak |
|---|---|---|
| `rate_dps` | `float` | Pan → Gyro veya Step Motor komutu; Tilt → Gyro |
| `absolute_deg` | `float` | Pan → Pusula (Compass); Tilt → İvmeölçer (Accel) |
| `dt` | `float` | Scheduler tick süresi |

| Output | Tip | Hedef |
|---|---|---|
| `estimate_deg` | `float` | `AxisManager::update()` → `current_position_deg` |

#### Matematiksel Formül

```
estimate = alpha * (estimate + rate * dt) + (1 - alpha) * absolute
```

- `alpha = 0.98` → %98 gyro/step, %2 pusula/accel
- Uyanışta (boot): `reset(absolute_deg)` → İlk değer %100 pusula/accel ile tohumlanır.

---

## Bağımlılıklar

Bu modülün diğer `ace/*` modüllerine bağımlılığı **yoktur**. Tamamen bağımsız matematik katmanıdır.
