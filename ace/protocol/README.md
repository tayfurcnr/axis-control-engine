# ace/protocol — ALP Protokol Ayrıştırıcısı

GCS veya başka bir kontrol birimi, UART/UDP üzerinden text tabanlı komutlar gönderir. `Protocol` modülü bu ham metni alır, doğrular, `CommandRequest` modeline çevirir ve `AxisManager`'a iletir.

---

## Veri Akışı

```
GCS / Seri Port
    │  "SET_ANGLE PAN 45.0\r\n"  (raw byte stream)
    ▼
AlpParser::parse(line)
    │  Token ayrıştırma, ID eşleştirme, parametre dönüşümü
    ▼
CommandRequest { command_id: SET_ANGLE, axis: PAN, target_angle_pan_deg: 45.0 }
    │
    ▼
ProtocolDispatcher::dispatch(request)
    │
    ▼
AxisManager::execute(request)
    │
    ▼
CommandOutcome { ACK / NACK }
    │
    ▼
AlpSerializer::serialize(outcome)  →  "ACK 0x20\r\n"  →  GCS
```

---

## Input / Output Tablosu

| Input | Tip | Kaynak |
|---|---|---|
| Ham komut satırı | `const char*` / `std::string_view` | UART RX / UDP socket |

| Output | Tip | Hedef |
|---|---|---|
| `CommandRequest` | Struct | `AxisManager::execute()` |
| Yanıt metni | `const char*` | UART TX / UDP socket → GCS |

---

## Desteklenen Komut Formatı

```
<KOMUT_ADI> [AXIS] [PARAM1] [PARAM2] ...\r\n
```

Örnekler:
```
SET_ANGLE PAN 45.0
SET_ANGLE TILT -10.0
SET_PID ALL 0.5 0.01 0.02 0.0 100.0 1.0
HOME ALL
FACTORY_RESET
SET_SERIAL ACE-PT-001A
```

---

## Hata Yönetimi

| Hata Durumu | Yanıt |
|---|---|
| Bilinmeyen komut | `NACK 0x00 0x03000401` |
| Geçersiz parametre tipi (sayı bekleniyordu) | `NACK <CMD_ID> 0x03000403` |
| Eksik parametre | `NACK <CMD_ID> 0x03000402` |
| Limit aşımı | `NACK <CMD_ID> 0x02000501` |

---

## Bağımlılıklar

| Modül | Neden? |
|---|---|
| `ace/communication` | `CommandRequest`, `CommandId` tipleri |
| `ace/axis` | `AxisManager::execute()` çağrısı |
