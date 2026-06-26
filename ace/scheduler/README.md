# Scheduler Modülü

İşletim sistemi (RTOS) bağlamındaki threadleri, timer mekanizmalarını kontrol eden modüldür. Cihaz içindeki ana AxisManager döngüsü (kontrol hesabı) `1ms` periyotta (1000 Hz) dönmesini bu bloğa borçludur. Aynı anda diğer eşzamanlı background işlemlerini (Telemetry gönderimi gibi ~20ms pencerelerde) aralara serpiştirerek CPU yükünü yönetir.
