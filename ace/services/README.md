# ace/services — Destek Servisleri (Logger)

Tüm modüllerin kullandığı genel amaçlı yardımcıları barındırır. Donanıma veya protokole bağımlı değildir; her yerden erişilebilir.

---

## Bileşenler

### `Logger` — Olay Kaydedici

Sistemin tüm modülleri, hata ve bilgilendirme mesajlarını `Logger` üzerinden yayar.

#### Veri Akışı

```
AxisManager / Controller / vb.
    │  log_debug("modül", "mesaj")
    ▼
Logger::log(level, area, message)
    │
    ├── [DEBUG ON]  → UART / stdout'a yazar  →  GCS ekranında görünür
    └── [DEBUG OFF] → Sessizce yutar
```

#### Input / Output

| Input | Tip | Kaynak |
|---|---|---|
| `level` | `LogLevel` (DEBUG/INFO/WARN/ERROR) | Çağıran modül |
| `area` | `const char*` | Çağıran modül (örn: `"axis.execute"`) |
| `message` | `const char*` | Çağıran modül |

| Output | Kaynak | Koşul |
|---|---|---|
| UART / stdout log satırı | Logger | `debug_enabled == true` |

#### Özellikler

- `SET_DEBUG 0/1` komutu ile runtime'da debug çıktısı açılıp kapatılabilir.
- ESP32 hedefinde çıktı `uart_write_bytes()` ile UART0'dan gider.
- PC testlerinde `stdout`'a yazar.

---

## Bağımlılıklar

Bu modülün diğer `ace/*` modüllerine bağımlılığı **yoktur**.
