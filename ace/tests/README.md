# ace/tests — Platform Bağımsız Test Ortamı

Proje kodunu ESP32 donanımına ihtiyaç duymadan standart `g++` ile PC'de doğrulayan birim (unit) testleri. Tüm testler `ace/tests/` altındaki `.cpp` dosyalarında bulunur.

---

## Test Listesi

| Dosya | Test Edilen Modül | Senaryo |
|---|---|---|
| `test_axis_manager.cpp` | `ace/axis` | Durum makinası geçişleri, IMU boot homing doğruluğu |
| `test_geometry.cpp` | `ace/geometry` | Coğrafi hedef projeksiyonu ve pan/tilt hesabı |
| `test_sensor_fusion.cpp` | `ace/control` | ComplementaryFilter ilk değer, füzyon oranı, drift düzeltme |

---

## Çalıştırma

Her test dosyası bağımsız olarak derlenir ve çalıştırılır:

```bash
# AxisManager testleri
g++ -std=c++17 \
  ace/tests/test_axis_manager.cpp \
  ace/axis/axis_manager.cpp \
  ace/control/controller.cpp \
  ace/control/sensor_fusion.cpp \
  ace/geometry/target_geometry.cpp \
  ace/motion/motion_planner.cpp \
  ace/telemetry/telemetry_publisher.cpp \
  ace/services/logger.cpp \
  -I. -o /tmp/test_axis && /tmp/test_axis

# Geometry testleri
g++ -std=c++17 \
  ace/tests/test_geometry.cpp \
  ace/geometry/target_geometry.cpp \
  -I. -o /tmp/test_geo && /tmp/test_geo

# SensorFusion testleri
g++ -std=c++17 \
  ace/tests/test_sensor_fusion.cpp \
  ace/control/sensor_fusion.cpp \
  -I. -o /tmp/test_sf && /tmp/test_sf
```

---

## Mock Yapısı

Test ortamı gerçek donanım yerine stub / mock nesneler kullanır:

| Gerçek Nesne | Mock Karşılığı |
|---|---|
| NVS (Flash) | `MockStorage` (bellekte, kayıt yapmaz) |
| IMU | `SensorData` struct manuel doldurularak verilir |
| UART | `stdout` |

---

## Yeni Test Yazma Kuralları

1. Dosya adı: `test_<modül_adı>.cpp`
2. Bağımsız bir `main()` içersin, framework gerektirmesin.
3. `assert()` kullanarak doğrulama yap, geçerse terminale `"<isim> passed."` yaz.
4. Mock'lar bu dizinde değil, doğrudan test dosyası içinde anonim `namespace` veya yerel sınıf olarak tanımlanabilir.

---

## Bağımlılıklar

Test dosyaları yalnızca test ettiği modüllere bağımlıdır. HAL/Drivers ve FreeRTOS **bağımlılığı yoktur**.
