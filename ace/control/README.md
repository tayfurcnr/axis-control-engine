# Control Modülü

`ace/control` paketi eksenlerin matematiksel sürücü mantığını oluşturur.
Sistemden ve donanımdan tamamen yalıtılmış olarak çalışan `Controller` sınıfını barındırır.

### Özellikler
- **PID Loop (Proportional-Integral-Derivative)**
- **Feed-Forward Desteği:** Sisteme ek bir baz itki uygulanması.
- **Integral Windup Koruması:** `max_integral` kısıtlaması sayesinde fren anında ani sapmaların engellenmesi.
- **Output Clamping:** Motor PWM sürücüsüne gönderilecek %100'lük sınırın (max_output) tork olarak kesilmesi.

Donanıma doğrudan bağlanmaz, sadece `AxisManager` tarafından her an (örneğin 1ms loop ile) `update` fonksiyonuyla çağrılarak o anki teorik `setpoint` ve sensör okuması (`measurement`) arasındaki düzeltme eforunu C++ matematik katmanı olarak üretir.
