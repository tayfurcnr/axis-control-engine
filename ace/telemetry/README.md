# Telemetry Modülü

`ace/telemetry` sistemi AxisManager ve diğer modüllerin o anki durumlarını, hatalarını, çalışma modlarını ve hareket/pozisyon ölçümlerini bir rapor (telemetri) dizesi olarak hazırlayıp göndermekle görevlidir.

### TelemetryPublisher Sınıfı
Ana veri oluşturucudur. `Scheduler` (Zamanlayıcı / Timer) tarafından genellikle saniyede 50-100 kez (20ms/10ms aralıklarla) vb. çağrılır.
- Dış sisteme `PROTOCOL_REFERENCE` şemasına %100 uyumlu text serileştirmesi yaparak, eksenlerin anlık PWM değerleri, pan/tilt encoder açıları gibi can alıcı bilgileri süzüp `TELEMETRY ENABLED=1 MOVING=0 PAN_ANGLE=...` standardıyla dışarı formatlayıp çıkarır.
