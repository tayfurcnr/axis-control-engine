# Axis Module

`AxisManager` eksenlerin mantıksal durumunu, komut işleme akışını ve event üretimini yönetir.

| Alan | Değer |
| --- | --- |
| Public API | `enable`, `disable`, `execute`, `consume_event`, `set_angle`, `set_velocity`, `stop`, `home`, `calibrate`, `set_mode`, `set_location`, `set_target`, `update` |
| Internal | `AxisStatus`, state geçişleri, pending event kuyruğu |
| Platform | Düşük |

## Bağımlılıklar

- `ace/communication`
- `ace/config`
- `ace/services`

## Platform Durumu

- Doğrudan GPIO/PWM/encoder kullanmaz.
- Platforma özel sürüş kodu burada değil, daha alt bir sürücü katmanında olmalıdır.

## Notlar

- Kalıcı konum bilgisi storage arayüzü üzerinden yönetilir.
- ACK/NACK ve EVENT akışı bu katmanda şekillenir.
- Açı, hız ve coğrafi limitler, motor PID döngü kazanç parametreleri (`kp, ki, kd`) `ace/config/PersistentConfig` üzerinden okunur (Hot-Reload edilebilir).
- `update(dt)` döngüsünde kapalı çevrim (Closed Loop) kontrolcüleri ve Trapezoidal (İvmeli) hareket planlayıcı sınıflarını çalıştırır.
- `SET_TARGET` koordinat ataması ile Enlem, Boylam ve İrtifa verilerinden Haversine ve Trigonometrik izdüşümle otomatik Pan ve Tilt açılarını belirleyerek (`MODE=TRACK`) takip başlatır.
