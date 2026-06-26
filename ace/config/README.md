# Config Modülü

`ace/config` sistemi, çalışma zamanı (Runtime) ve kalıcı (Persistent/NVS) değişkenlerin yönetim birimidir.

- **PersistentStorage:** ESP32 Non-Volatile Storage (NVS) API kullanarak sensör, ivme, limit ve pid ayarlarını flash diske yazar ve okur.
- **Config Modelleri:** `PidConfig`, `MotionConfig`, `AxisLimitConfig` gibi cihaz genelinde AxisManager'in sınırlarını çizen struct haritalarını tutar.
- **Hot-Reload:** Cihaz çalışırken `SET_PID` gibi güncellemeler gelirse modüller resetlenmeden değerler anında sisteme bind edilir. NVS verisyonlaması kVersion sayesinde geriye dönük güvenliği de otomatik kendi halleder.
