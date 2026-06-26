# Modules Overview

Bu dosya, repodaki ana modüllerin ne yaptığını ve hangi katmanlara bağımlı olduğunu hızlıca gösterir.

| Module | Purpose | Platform Dependency |
| --- | --- | --- |
| `ace/axis` | Eksen durumları, komut işleme, ACK/NACK ve EVENT akışı | Düşük |
| `ace/communication` | Komut kimlikleri, mesaj modelleri, protokol veri tipleri | Yok |
| `ace/config` | Runtime ve kalıcı konfigürasyon | Orta |
| `ace/control` | Üst seviye kontrol kararları | Düşük |
| `ace/diagnostics` | Self-test ve sağlık kontrolleri | Düşük |
| `ace/drivers` | Donanım sürücüleri | Yüksek |
| `ace/hal` | Donanım soyutlama katmanı | Yüksek |
| `ace/motion` | Hareket planlama ve trajectory mantığı | Düşük |
| `ace/protocol` | Dış mesajların domain katmanına yönlendirilmesi | Düşük |
| `ace/safety` | Güvenli duruma geçiş ve emniyet kuralları | Düşük |
| `ace/scheduler` | RTOS task ve loop organizasyonu | Yüksek |
| `ace/services` | Logger ve benzeri servisler | Orta |
| `ace/telemetry` | Durum ve telemetri üretimi | Düşük |

## Bağımlılık Notu

- `Yok`: modül platforma özel API kullanmamalı.
- `Düşük`: platformdan bağımsız kalması hedeflenen ama üst katmanlarla konuşan modül.
- `Orta`: storage veya servis implementasyonu nedeniyle sınırlı platform bağımlılığı olan modül.
- `Yüksek`: doğrudan ESP/RTOS/HAL/donanım API'lerine bağlı modül.

## İlgili Dokümanlar

- [Protocol Reference](./PROTOCOL_REFERENCE.md)
- [Axis Module](./ace/axis/README.md)
- [Config Module](./ace/config/README.md)
- [Communication Module](./ace/communication/README.md)
