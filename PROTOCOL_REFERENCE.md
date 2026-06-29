# Axis Control Engine (ACE)

ACE, ESP32 tabanlı çok eksenli hareket kontrol yazılımıdır.

## Product Identity

| Field | Value |
| --- | --- |
| Device Name | `ACE-PT-01` |
| Device ID | `0xACE1` |

## Command IDs

Komutlar fonksiyonel gruplara ayrılmıştır. Bu yapı hem okunabilirliği artırır hem de protokol genişlerken çakışmayı azaltır.

| Group | Command | ID |
| --- | --- | --- |
| System | `ENABLE` | `0x10` |
| System | `DISABLE` | `0x11` |
| System | `STOP` | `0x12` |
| Motion | `SET_ANGLE` | `0x20` |
| Motion | `SET_VELOCITY` | `0x21` |
| Motion | `HOME` | `0x22` |
| Motion | `SET_TARGET` | `0x23` |
| Motion | `SET_MODE` | `0x24` |
| Maintenance | `CALIBRATE` | `0x30` |
| Configuration | `SET_PID` | `0x31` |
| Configuration | `SET_MOTION_LIMITS` | `0x32` |
| Telemetry | `GET_TELEMETRY` | `0x40` |
| Heartbeat | `HEARTBEAT` | `0x50` |
| System | `SET_LOCATION` | `0x60` |
| System | `SET_DEBUG` | `0x61` |
| System | `SET_SERIAL` | `0x62` |
| Telemetry | `ERROR` | `0x70` |
| System | `GET_INFO` | `0x80` |
| System | `REBOOT` | `0x90` |
| System | `FACTORY_RESET` | `0x91` |

## Command Meaning

- Axis selector IDs:

| Axis | ID | Meaning |
| --- | --- | --- |
| `ALL` | `0x00` | Her iki eksen, wildcard |
| `PAN` | `0x01` | Pan ekseni |
| `TILT` | `0x02` | Tilt ekseni |

- `ENABLE`: Eksenleri çalışmaya hazır hale getirir. Servo/motor tarafını aktif duruma alır.
- `DISABLE`: Eksenleri devre dışı bırakır. Motor çıkışını kapatır ve güvenli duruma çeker.
- `STOP`: O anki hareketi durdurur, hız komutunu sıfırlar.
- `SET_ANGLE`: Pan ve tilt eksenlerini hedef açıya götürür.
- `SET_VELOCITY`: Pan ve tilt eksenlerine hedef hız verir.
- `HOME`: Eksenleri referans/home noktasına götürür.
- `SET_TARGET`: Cihazın bakacağı hedef konumu `longitude`, `latitude` ve deniz seviyesi yüksekliği ile tanımlar.
- `SET_MODE`: Cihazın çalışma modunu seçer.
- `CALIBRATE`: Kalibrasyon sürecini başlatır.
- `SET_PID`: Seçili eksenin çalışma sırasındaki PID parametrelerini günceller ve kalıcı diske kaydeder.
- `SET_MOTION_LIMITS`: Seçili eksenin maksimum hız ve ivmelenme limitlerini günceller ve kalıcı diske kaydeder.
- `GET_TELEMETRY`: Genişletilmiş durum bilgisini okur.
- `HEARTBEAT`: Bağlantının canlı olduğunu doğrulayan periyodik işarettir.
- `SET_LOCATION`: Cihazın yerleştirileceği konumu `longitude`, `latitude` ve deniz seviyesi yüksekliği ile tanımlar.
- `SET_DEBUG`: Terminal debug çıktısını açar veya kapatır.
- `ERROR`: Sistem tarafından üretilen hata mesajıdır. Yalnızca hata kodu taşır; ek veri varsa protokolün ilgili sürümünde opsiyonel alan olarak eklenebilir.
- `GET_INFO`: Firmware, cihaz ve protokol sürüm bilgisini verir.
- `REBOOT`: Cihazı yeniden başlatır.
- `FACTORY_RESET`: Kalibrasyon, konfigürasyon ve kullanıcı verilerini fabrika ayarlarına döndürür.

## Protocol Reference

Bu bölüm, komutlara verilen senkron cevapları ve cihazın yayınladığı asenkron mesaj tiplerini içerir.

### Responses

#### `TELEMETRY`

```text
TELEMETRY ENABLED=<0|1> MOVING=<0|1> PAN_ANGLE=<DEG> TILT_ANGLE=<DEG> PAN_VELOCITY=<DPS> TILT_VELOCITY=<DPS> TEMPERATURE=<C> VOLTAGE=<V> MODE=<MODE> ERROR_CODE=<HEX>
```

- `ENABLED`: eksenlerin aktif olup olmadığını bildirir
- `MOVING`: hareket halinde olup olmadığını bildirir
- `PAN_ANGLE`: anlık pan açısı
- `TILT_ANGLE`: anlık tilt açısı
- `PAN_VELOCITY`: anlık pan hızı
- `TILT_VELOCITY`: anlık tilt hızı
- `TEMPERATURE`: sürücü veya sistem sıcaklığı
- `VOLTAGE`: besleme gerilimi
- `MODE`: aktif çalışma modu
- `ERROR_CODE`: aktif hata kodu, varsa
- Bu cevap `GET_TELEMETRY` komutuna karşılık gelir

#### `INFO`

```text
INFO DEVICE_NAME=<NAME> DEVICE_ID=<HEX> FW_VERSION=<SEMVER> PROTOCOL_VERSION=<SEMVER> BUILD_DATE=<YYYY-MM-DD> BUILD_TIME=<HH:MM:SS> BOARD=<BOARD> FEATURES=<CSV>
```

- `DEVICE_NAME`: cihaz adı
- `DEVICE_ID`: cihaz kimliği
- `FW_VERSION`: firmware sürümü
- `PROTOCOL_VERSION`: protokol sürümü
- `BUILD_DATE`: derleme tarihi
- `BUILD_TIME`: derleme saati
- `BOARD`: hedef donanım kartı
- `FEATURES`: virgülle ayrılmış özellik listesi
- Bu cevap `GET_INFO` komutuna karşılık gelir

#### `EVENT`

```text
EVENT <EVENT_TYPE> <CMD_ID>
```

- `EVENT_TYPE`: olay tipi etiketi veya onun ID karşılığı
- `CMD_ID`: tamamlanan komutun ID değeri
- Komut cevabı değildir, asenkron olay bildirimi olarak kullanılır
- Hareket tamamlandığında veya hedefe ulaşıldığında bu tek çerçeve gönderilir

#### `ACK`

```text
ACK <CMD_ID>
```

- `CMD_ID`: gelen ve işlenen komutun ID değeri
- Komutun başarıyla kabul edilip işleme alındığını bildirir
- Hareket komutlarında fiziksel hareketin tamamlandığı anlamına gelmez
- Hareket tamamlandığında asenkron bir `EVENT` çerçevesi gönderilir
- `ACK` her zaman orijinal komutun ID değerini geri taşır

Örnek:

```text
ACK 0x20
```

#### `NACK`

```text
NACK <CMD_ID> <ERROR_CODE>
```

- `CMD_ID`: başarısız olan komutun ID değeri
- `ERROR_CODE`: birleşik hata kodu
- Komut başarısız olduğunda gönderilir
- `NACK` her zaman orijinal komutun ID değerini geri taşır

Örnek:

```text
NACK 0x23 <ERROR_CODE>
```

- `ERROR_CODE`, aşağıdaki `Error Code Layout` tablosundaki alanlardan türetilir
- Busy durumu ayrı bir protokol türü olarak değil, normal hata kodu olarak taşınır

### Units

- `ANGLE`: derece
- `VELOCITY`: derece/saniye
- `LONGITUDE`: derece
- `LATITUDE`: derece
- `ALTITUDE_M`: metre
- `ERROR_CODE`: `uint32`

## Persistent Data

Kalıcı saklama için EEPROM yerine ESP32 NVS tercih edilmelidir.

### Kalıcı Olarak Saklanabilir Alanlar

| Field | Store | Reason |
| --- | --- | --- |
| `SET_LOCATION` | Evet | Cihazın kurulu olduğu konum açılışta yeniden kullanılabilir |
| Kalibrasyon offsetleri | Evet | Home/zero referansı tekrar yüklenebilir |
| Eksen yönü / invert ayarları | Evet | Donanım montajına bağlı sabit konfigürasyon |
| Soft limit değerleri | Evet | Güvenlik ve hareket sınırları için kalıcı konfigürasyon |
| Son mod bilgisi | Opsiyonel | İstenirse açılışta varsayılan mod olarak uygulanabilir |

### Kalıcı Olarak Saklanmaması Gereken Alanlar

| Field | Store | Reason |
| --- | --- | --- |
| `SET_TARGET` | Hayır | Geçici görev/hedef bilgisidir |
| `current_position` | Hayır | Anlık runtime durumudur |
| `current_velocity` | Hayır | Anlık runtime durumudur |
| `moving` | Hayır | Anlık runtime durumudur |
| `faulted` | Hayır | Sistem yeniden başlatıldığında yeniden hesaplanmalıdır |

## Protocol Events

Bu event'ler komut cevabı değildir. `ACK/NACK` mekanizmasından bağımsız olarak, komut tamamlandığında asenkron biçimde `EVENT` çerçevesi yayınlanır.

### Event Types

| Event Type | ID | Meaning |
| --- | --- | --- |
| `MOTION_DONE` | `0x01` | Hareket komutu fiziksel olarak tamamlandı |
| `TARGET_REACHED` | `0x02` | Hedefe bakma işlemi tamamlandı |

## Info Fields

`GET_INFO` cevabında aşağıdaki alanlar bulunur:

- `DEVICE_NAME`
- `DEVICE_ID`
- `FW_VERSION`
- `PROTOCOL_VERSION`
- `BUILD_DATE`
- `BUILD_TIME`
- `BOARD`
- `FEATURES`

Örnek:

```text
INFO DEVICE_NAME=ACE-PT-01 DEVICE_ID=0xACE1 FW_VERSION=1.0.0 PROTOCOL_VERSION=0.1.0 BUILD_DATE=2026-06-25 BUILD_TIME=12:00:00 BOARD=ESP32-S3 FEATURES=PAN,TILT,LOCATION,TARGET,TELEMETRY
```

### Mode IDs

| Mode | ID | Meaning |
| --- | --- | --- |
| `MANUAL` | `0x01` | Doğrudan komutla (SET_ANGLE, SET_VELOCITY) yönlendirme modu |
| `AUTO` | `0x02` | Dahili Planner tarafından çizilen rotada (Örn: 360 tarama alanı) sistemin çalıştırıldığı mod |
| `TRACK` | `0x03` | Verilen coğrafi (Enlem, Boylam) kilitlenmiş hedefi takip etme modu |

### Capability Matrix (Mode Based Access Control)

Kullanıcının seçtiği aktif operasyonel mod (`ModeId`), sistemin dışarıdan kabul edebileceği hareket komutlarını doğrudan etkiler. Bu mantık, `TRACK` modunda hedefe kitlenmişken kullanıcının sehven göndereceği joystick/açı komutlarının takip algoritmasını bozmaması için tasarlanmıştır. Belirli modlarda belirli komutlar reddedilir (`NACK`).

| Command | `MANUAL` Mod Gerekli Mi? | Desteklenen Modlar | Açıklama |
| --- | --- | --- | --- |
| `SET_ANGLE` | Evet | Yalnızca `MANUAL` | İstenilen doğrudan açıya gider. `TRACK`/`AUTO`'da `NACK` döner. |
| `SET_VELOCITY` | Evet | Yalnızca `MANUAL` | İstenilen doğrudan hızda döner. `TRACK`/`AUTO`'da `NACK` döner. |
| `SET_TARGET` | Hayır | `TRACK`, `AUTO` | `AUTO` modunda hedef saklanır, `TRACK` modunda hedef saklanır ve geometri hesabı ile yönlendirme başlar. `MANUAL` modda `NACK` döner. |
| `SET_MODE` | Hayır | Tümü | `TRACK` seçilmeden önce cihaz konumu tanımlı olmalıdır; aksi durumda `NACK <CMD_ID> 0x03000403` döner. |
| `CALIBRATE`, `HOME` | İhtiyari | (Tümü) | Gelişmiş güvenlikte sadece MANUAL'e kilitlenebilir, şimdilik serbesttir. |
| `ENABLE`, `STOP`, `SET_PID` vb. | Hayır | Tümü | Sistem yönetimi ve acil durum komutları HER modda daima çalışır. |


### Stop Type IDs

| Stop Type | ID | Meaning |
| --- | --- | --- |
| `SOFT` | `0x01` | Kontrollü yavaşlama ile duruş |
| `HARD` | `0x02` | Hızlı kontrollü duruş |
| `EMERGENCY` | `0x03` | Acil ve öncelikli duruş |

### `ENABLE`

```text
ENABLE
```

- Parametre almaz.
- Eksenleri çalışmaya hazır duruma geçirir.

### `DISABLE`

```text
DISABLE
```

- Parametre almaz.
- Eksenleri devre dışı bırakır.

### `STOP`

```text
STOP <TYPE>
```

- `TYPE`: `SOFT`, `HARD`, veya `EMERGENCY`
- `TYPE` ID karşılıkları:
  - `SOFT` -> `0x01`
  - `HARD` -> `0x02`
  - `EMERGENCY` -> `0x03`
- `SOFT`: kontrollü yavaşlama ile durdurur
- `HARD`: hızlı duruş uygular
- `EMERGENCY`: mümkün olan en hızlı ve güvenli acil duruşu uygular
- Geçersiz `TYPE` için `NACK <CMD_ID> 0x03000403` döner

Örnek:

```text
STOP SOFT
STOP HARD
STOP EMERGENCY
```

### `SET_ANGLE`

```text
SET_ANGLE <AXIS> <ANGLE>
SET_ANGLE ALL <PAN_ANGLE> <TILT_ANGLE>
```

- `AXIS`: `PAN`, `TILT`, veya `ALL`
- `ANGLE`: hedef açı, derece cinsinden
- `PAN_ANGLE`: pan ekseni için hedef açı
- `TILT_ANGLE`: tilt ekseni için hedef açı
- `ALL` kullanılırsa iki eksen için de açı verilmelidir
- `AXIS` geçersizse veya açı sınır dışı ise `NACK <CMD_ID> 0x03000403` ya da `NACK <CMD_ID> 0x03000203` döner

Örnek:

```text
SET_ANGLE PAN 10
SET_ANGLE TILT 15
SET_ANGLE ALL 10 15
```

### `SET_VELOCITY`

```text
SET_VELOCITY <AXIS> <VELOCITY>
SET_VELOCITY ALL <PAN_VELOCITY> <TILT_VELOCITY>
```

- `AXIS`: `PAN`, `TILT`, veya `ALL`
- `VELOCITY`: hedef hız, derece/saniye cinsinden
- `PAN_VELOCITY`: pan ekseni için hedef hız
- `TILT_VELOCITY`: tilt ekseni için hedef hız
- `ALL` kullanılırsa iki eksen için de hız verilmelidir
- `AXIS` geçersizse veya hız sınır dışı ise `NACK <CMD_ID> 0x03000403` ya da `NACK <CMD_ID> 0x03000203` döner

Örnek:

```text
SET_VELOCITY PAN 20
SET_VELOCITY TILT 12
SET_VELOCITY ALL 20 12
```

### `HOME`

```text
HOME <AXIS>
```

- `AXIS`: `PAN`, `TILT`, veya `ALL`
- Seçilen ekseni home noktasına götürür
- Geçersiz `AXIS` için `NACK <CMD_ID> 0x03000403` döner

Örnek:

```text
HOME ALL
HOME PAN
HOME TILT
```

### `SET_MODE`

```text
SET_MODE <MODE>
```

- `MODE`: `MANUAL`, `AUTO`, veya `TRACK`
- `MANUAL`: eksenler doğrudan açı/hız komutlarıyla yönetilir
- `AUTO`: iç kontrol ve planlama mantığı aktif olur
- `TRACK`: `SET_TARGET` üzerinden hedef takibi yapar
- `TRACK` seçimi için cihaz konumu önceden `SET_LOCATION` ile tanımlanmış olmalıdır
- Geçersiz `MODE` için `NACK <CMD_ID> 0x03000403` döner

Örnek:

```text
SET_MODE MANUAL
SET_MODE AUTO
SET_MODE TRACK
```

### `SET_PID`

```text
SET_PID <AXIS> <KP> <KI> <KD> <FF> <MAX_INT> <MAX_OUT>
```

- `AXIS`: `PAN`, `TILT`, veya `ALL`
- Parametreler `float` formatında ondalıklı girilir.
- O an çalışan PID bloğunu anında ("hot-reload") günceller.
- Başarılı olursa NVS diske kayıt yaparak kalıcılaştırır, cihaz resetlense bile ayar korunur.

### `SET_MOTION_LIMITS`

```text
SET_MOTION_LIMITS <AXIS> <MAX_VEL> <MAX_ACCEL>
```

- `AXIS`: `PAN`, `TILT`, veya `ALL`
- Parametreler `float` formatında girilir (Dps ve Dps²).
- Başarılı olursa hareket planlayıcısına anlık uygulanır ve NVS bellek kalıcı olarak güncellenir.


### `SET_LOCATION`

```text
SET_LOCATION <LONGITUDE> <LATITUDE> <ALTITUDE_M>
```

- `LONGITUDE`: cihazın boylamı, derece cinsinden
- `LATITUDE`: cihazın enlemi, derece cinsinden
- `ALTITUDE_M`: deniz seviyesine göre yükseklik, metre cinsinden
- Bu komut cihazın kurulu olduğu coğrafi konumu tanımlar
- `LONGITUDE` ve `LATITUDE` değerleri `double` hassasiyetinde gönderilmelidir
- `LONGITUDE` ve `LATITUDE` için nokta sonrası en az 7 hane önerilir
- `ALTITUDE_M` `double` olabilir; metre cinsinden yükseklik bilgisini taşır
- `LONGITUDE`, `LATITUDE` veya `ALTITUDE_M` sınır dışı ise `NACK <CMD_ID> 0x03000203` döner

Örnek:

```text
SET_LOCATION 29.0123 41.0082 125.0
```

### `SET_DEBUG`

```text
SET_DEBUG <0|1>
```

- `0`: debug çıktısını kapatır
- `1`: debug çıktısını açar
- Bu komut terminale basılan alan bazlı debug mesajlarını yönetir
- Komut kabul edilirse `ACK <CMD_ID>` döner
- Parametre geçersizse `NACK <CMD_ID> 0x03000403` döner

Örnek:

```text
SET_DEBUG 1
SET_DEBUG 0
```

### `SET_SERIAL`

```text
SET_SERIAL <SERIAL_NUMBER>
```

- `SERIAL_NUMBER`: cihazın benzersiz kalıcı seri numarası (en fazla 31 karakter, boşluk içermemeli)
- Cihazın NVS belleğinde saklanan seri numarasını günceller
- `FACTORY_RESET` işlemi bu alanı SİLMEZ. Seri numarası donanım kimliği olarak korunur.
- Hiç ayarlanmamışsa, `device_config.hpp` içindeki varsayılan değer döner.
- Komut kabul edilirse `ACK <CMD_ID>` döner.

Örnek:

```text
SET_SERIAL SN-ACE-01-A
```

### `SET_TARGET`

```text
SET_TARGET <TARGET_LONGITUDE> <TARGET_LATITUDE> <TARGET_ALTITUDE_M>
```

- `TARGET_LONGITUDE`: hedef noktanın boylamı, derece cinsinden
- `TARGET_LATITUDE`: hedef noktanın enlemi, derece cinsinden
- `TARGET_ALTITUDE_M`: hedef noktanın deniz seviyesine göre yüksekliği, metre cinsinden
- `AUTO` modunda hedef saklanır; `TRACK` modunda hedef saklanır ve cihazın bakacağı pan ve tilt açıları hesaplanır
- `MANUAL` modda bu komut kabul edilmez ve `NACK` döner
- `TARGET_LONGITUDE` ve `TARGET_LATITUDE` değerleri `double` hassasiyetinde gönderilmelidir
- `TARGET_LONGITUDE` ve `TARGET_LATITUDE` için nokta sonrası en az 7 hane önerilir
- `TARGET_LONGITUDE`, `TARGET_LATITUDE` veya `TARGET_ALTITUDE_M` sınır dışı ise `NACK <CMD_ID> 0x03000203` döner

Örnek:

```text
SET_TARGET 29.0130000 41.0130000 130.0
```

### Limits / Validation

#### Axis Limits

- `PAN` angle range: `-180.0` to `180.0` degree
- `TILT` angle range: `-90.0` to `90.0` degree
- `ALL` values must individually satisfy the selected axis limits

#### Velocity Limits

- `PAN` velocity range: `-60.0` to `60.0` degree/s
- `TILT` velocity range: `-60.0` to `60.0` degree/s
- `0.0` means stop for velocity commands

#### Location Limits

- `LONGITUDE`: `-180.0` to `180.0`
- `LATITUDE`: `-90.0` to `90.0`
- `ALTITUDE_M`: `-500.0` to `9000.0`

#### Target Limits

- `TARGET_LONGITUDE`: `-180.0` to `180.0`
- `TARGET_LATITUDE`: `-90.0` to `90.0`
- `TARGET_ALTITUDE_M`: `-500.0` to `9000.0`
- Target coordinates must be valid before conversion to pan/tilt angles

#### `CALIBRATE`

```text
CALIBRATE
```

- Parametre almaz.
- Kalibrasyon sürecini başlatır.

#### `GET_TELEMETRY`

```text
GET_TELEMETRY
```

- Parametre almaz.
- Cihazın genişletilmiş telemetri paketini okur.
- Dönen alanlar:
  - `ENABLED`
  - `MOVING`
  - `PAN_ANGLE`
  - `TILT_ANGLE`
  - `PAN_VELOCITY`
  - `TILT_VELOCITY`
  - `TEMPERATURE`
  - `VOLTAGE`
  - `MODE`
  - `ERROR_CODE`

Örnek cevap:

```text
TELEMETRY ENABLED=1 MOVING=0 PAN_ANGLE=10.0 TILT_ANGLE=15.0 PAN_VELOCITY=0.0 TILT_VELOCITY=0.0 TEMPERATURE=42.5 VOLTAGE=24.1 MODE=AUTO ERROR_CODE=0x00000000
```

#### `HEARTBEAT`

```text
HEARTBEAT
```

- Parametre almaz.
- Canlı bağlantı işareti olarak kullanılır.

#### `GET_INFO`

```text
GET_INFO
```

- Parametre almaz.
- Firmware, cihaz ve protokol sürüm bilgisini okur.
- Cevap olarak `INFO` mesajı döner.

#### `REBOOT`

```text
REBOOT
```

- Parametre almaz.
- Cihazı yeniden başlatır.

#### `FACTORY_RESET`

```text
FACTORY_RESET
```

- Parametre almaz.
- Kalibrasyon ve yapılandırma verilerini fabrika ayarlarına döndürür.
- Onay gerektiren yüksek riskli işlemdir.

#### `ERROR`

```text
ERROR <ERROR_CODE>
```

- `ERROR_CODE`: birleşik hata kodu, `uint32`
- Hata kodu şu alanlardan oluşur: `GROUP_ID`, `MODULE_ID`, `REASON_ID`, `SEVERITY_ID`
- Her alan 1 bayt olarak paketlenir ve okunabilirlik için sözlük tabloları aşağıda tanımlanır
- Tüm birleşik hex örnekleri burada tek tek listelenmez; kod tarafı bu alanları `pack()` mantığıyla üretir

##### Error Code Layout

Hata kodu, alt başlıklar birleştirilerek üretilir.

```text
ERROR_CODE = <GROUP_ID><MODULE_ID><REASON_ID><SEVERITY_ID>
```

- `GROUP_ID`: hata grubunu belirtir
- `MODULE_ID`: hangi modülde oluştuğunu belirtir
- `REASON_ID`: neden kodunu belirtir
- `SEVERITY_ID`: hata seviyesi belirtir

##### Common Error Patterns

| Pattern | Meaning |
| --- | --- |
| `COMMUNICATION / SYSTEM / INVALID_PARAMETER / ERROR` | Komut veya parametre formatı geçersiz |
| `COMMUNICATION / SYSTEM / INVALID_STATE / ERROR` | Komut sistemin mevcut durumuyla uyumsuz |
| `COMMUNICATION / SYSTEM / LIMIT / ERROR` | Girilen değer sınır dışında |
| `MOTION / SYSTEM / BUSY / WARNING` | Sistem geçici olarak meşgul |
| `MOTION / SYSTEM / TIMEOUT / ERROR` | Hareket beklenen sürede tamamlanmadı |
| `STORAGE / NVS / NOT_INITIALIZED / ERROR` | Kalıcı saklama katmanı başlatılamadı |
| `STORAGE / NVS / READ_FAIL / ERROR` | Kalıcı saklama okuma işlemi başarısız |
| `STORAGE / NVS / WRITE_FAIL / ERROR` | Kalıcı saklama yazma işlemi başarısız |
| `STORAGE / NVS / VERSION_MISMATCH / ERROR` | Saklanan veri sürümü uyumsuz |
| `COMMUNICATION / SYSTEM / UNSUPPORTED / ERROR` | Komut veya özellik desteklenmiyor |

##### Group IDs

| Group | ID |
| --- | --- |
| Motion | `0x01` |
| Sensor | `0x02` |
| Communication | `0x03` |
| Power | `0x04` |
| Storage | `0x05` |

##### Module IDs

| Module | ID |
| --- | --- |
| `SYSTEM` | `0x00` |
| `PAN` | `0x01` |
| `TILT` | `0x02` |
| `IMU` | `0x03` |
| `ENCODER` | `0x04` |
| `COMM` | `0x05` |
| `GPS` | `0x06` |
| `FLASH` | `0x07` |
| `NVS` | `0x08` |
| `WATCHDOG` | `0x09` |
| `POWER_SUPPLY` | `0x0A` |

##### Reason IDs

| Reason | ID |
| --- | --- |
| `TIMEOUT` | `0x01` |
| `LIMIT` | `0x02` |
| `CRC` | `0x03` |
| `INVALID_PARAMETER` | `0x04` |
| `OVERCURRENT` | `0x05` |
| `BUSY` | `0x06` |
| `NOT_INITIALIZED` | `0x07` |
| `READ_FAIL` | `0x08` |
| `WRITE_FAIL` | `0x09` |
| `VERSION_MISMATCH` | `0x0A` |
| `UNSUPPORTED` | `0x0B` |
| `INVALID_STATE` | `0x0C` |

##### Severity IDs

| Severity | ID |
| --- | --- |
| `INFO` | `0x01` |
| `WARNING` | `0x02` |
| `ERROR` | `0x03` |
| `FATAL` | `0x04` |

Örnek:

```text
ERROR 0x02030101
```

- `0x02030101`: `GROUP=Sensor`, `MODULE=IMU`, `REASON=TIMEOUT`, `SEVERITY=INFO`

## Protocol Notes

- `Device ID` cihaz ailesini temsil eder.
- `Command ID` komutun hangi gruba ait olduğunu da kod değeri üzerinden gösterir.
- `Axis ID` hangi eksenin hedeflendiğini belirtir.
- `CommandId`, `AxisId`, `ModeId`, `StopTypeId` ve mesaj modelleri kod tarafında `ace/communication/command_ids.hpp` ile `ace/communication/message_models.hpp` içinde tutulur.

## Repository Layout

```text
ace/
├── axis/
├── communication/
├── config/
├── control/
├── drivers/
├── hal/
├── motion/
├── services/
├── tests/
├── docs/
└── ...
```
