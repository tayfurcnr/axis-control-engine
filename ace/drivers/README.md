# Drivers Modülü

Doğrudan ESP32 donanımsal çip özelliklerini yönetecek sürücü kodlarıdır. Motor sürmek için `mcpwm` donanım arayüzünü, tekerlek açılarını okumak için de `pcnt` pulse okuyucu API'sini barındırır. Yazılım mantığı ile saf donanım arasındaki en alt (low-level) katmandır.
