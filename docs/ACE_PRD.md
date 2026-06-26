# ACE
## Axis Control Engine
## Software Architecture
## Product Requirements Specification (PRS)
## Firmware Design Document (FDD)

**Version:** 1.0  
**Status:** Draft  
**Target Platform:** ESP32-S3  
**Framework:** ESP-IDF  
**Tooling:** PlatformIO  
**Build System:** CMake  
**Language:** C++17  
**RTOS:** FreeRTOS  
**Protocol:** ALP (Addressed Link Protocol)  
**Author:** Tayfur Çınar

## Revision History

| Version | Date | Description |
| --- | --- | --- |
| 0.1 | Initial Draft | Initial Architecture |
| 0.5 | Internal Review | Software Modules |
| 1.0 | Release | First Public Architecture |

## Table of Contents

1. Product Overview
2. Vision
3. Scope
4. Design Goals
5. Functional Requirements
6. Non-Functional Requirements
7. System Architecture
8. Software Layers
9. Component Architecture
10. Core Software Modules
11. Design Principles
12. Core Architecture
13. Axis Framework
14. Motion Engine
15. Motion Planner
16. Controller Engine
17. Position Controller
18. Velocity Controller
19. State Machine
20. Fault Manager
21. Safety Manager
22. Calibration Manager
23. Communication Architecture (ALP)
24. Directory Structure
25. Coding Standards
26. Development Roadmap
27. Acceptance Criteria
28. Future Roadmap
29. Internal Event Bus
30. Message Queue
31. Scheduler
32. RTOS Timing
33. Inter Module Communication
34. Object Model

## 1. Product Overview

### 1.1 Introduction

ACE (Axis Control Engine), ESP32 tabanlı, gerçek zamanlı çalışan, modüler ve genişletilebilir bir hareket kontrol yazılımıdır.

ACE, hassas konum kontrolü gerektiren iki eksenli veya çok eksenli sistemler için geliştirilmektedir.

Temel amacı:

- donanımdan bağımsız
- gerçek zamanlı
- yeniden kullanılabilir
- profesyonel

bir motion control framework oluşturmaktır.

### 1.2 Purpose

ACE aşağıdaki sistemlerde kullanılacaktır:

- Antenna Tracker
- EO/IR Gimbal
- AI Vision Tracking
- Pan-Tilt Platform
- Camera Systems
- Robotic Positioners
- Sensor Positioning Systems
- Future Multi Axis Motion Systems

### 1.3 Product Philosophy

ACE yalnızca bir pan-tilt firmware'i değildir.

ACE, bir **Motion Control Framework** olarak tasarlanacaktır.

Pan-Tilt yalnızca ilk uygulamadır.

Gelecekte aynı çekirdek ile aşağıdaki sistemler desteklenecektir:

- 3 Axis
- 4 Axis
- Robot Arm
- Linear Motion
- Servo Platform

## 2. Vision

### Vision Statement

ACE;

- gerçek zamanlı çalışan
- modüler
- donanımdan bağımsız
- yüksek hassasiyetli
- endüstriyel seviyede

bir motion control platformu olacaktır.

### Long Term Vision

Tek firmware farklı donanımlar üzerinde çalışabilmelidir.

Örnek hedef yapı taşları:

- ESP32 -> Stepper -> Absolute Encoder
- STM32 -> Servo -> Absolute Encoder
- Linux -> CANOpen Servo -> EtherCAT

ACE değiştirilmeden çalışmalıdır.

## 3. Scope

### Current Scope

İlk sürüm aşağıdaki özellikleri kapsar.

#### Motion

- 2 Axis
- Position Control
- Velocity Control
- Motion Planning
- Encoder Feedback

#### Hardware

- ESP32-S3
- Step Motor
- Absolute Encoder
- UART
- SPI
- GPIO

#### Communication

- ALP Protocol
- UART
- USB CDC
- RS485
- CAN (Future)

### Out of Scope

İlk sürümde bulunmayacak:

- EtherCAT
- CANOpen
- Ethernet
- OTA
- Lua
- Multi Controller Synchronization
- Distributed Motion

## 4. Design Goals

ACE aşağıdaki prensiplerle geliştirilecektir.

### Goal 1 - Real Time Control

Control Loop hiçbir haberleşmeden etkilenmeyecektir.

### Goal 2 - Hardware Independent

Encoder değişirse Motion Planner değişmeyecektir.

### Goal 3 - Reusable

Axis kodu Pan, Tilt ve Robot Arm için aynı olacaktır.

### Goal 4 - Deterministic

Her kontrol çevrimi aynı sürede tamamlanacaktır.

### Goal 5 - Scalable

2 Axis -> 3 Axis -> 6 Axis aynı mimari ile çalışacaktır.

## 5. Functional Requirements

ACE aşağıdaki fonksiyonları sağlayacaktır.

### Motion

- Position Control
- Velocity Control
- Motion Planning
- Soft Stop
- Emergency Stop
- Motion Queue

### Encoder

- Absolute Position
- Offset
- Health Monitoring

### Calibration

- Calibration

### Stepper

- STEP
- DIR
- Enable
- Disable
- Motion Generation

### Configuration

- Runtime Parameter Update
- Persistent Storage
- Factory Reset

### Communication

- ALP
- Telemetry
- Command
- Heartbeat

### Diagnostics

- Health Status
- Fault Codes
- Warning Codes
- Self Test

## 6. Non-Functional Requirements

ACE aşağıdaki kalite gereksinimlerini karşılayacaktır.

### Real Time

- Control Loop: 1000 Hz

### Reliable

- Encoder Loss -> Safe State

### Portable

HAL sayesinde platform geçişi kolay olacaktır:

- ESP32
- STM32
- Linux

### Testable

Her modül bağımsız test edilebilir olacaktır.

### Modular

Her modül yalnızca tek sorumluluğa sahip olacaktır.

## 7. System Architecture

### High Level Architecture

```text
Startup
        |
Axis Manager
        |
Axis Framework
        |
Motion Planner + Controller
        |
Hardware Abstraction
        |
Drivers (Encoder / Stepper)
        |
ESP32 Hardware
```

## 8. Software Layers

ACE toplam 6 katmandan oluşacaktır:

- Startup
- Axis Manager
- Services
- Axis
- Control
- HAL
- Drivers

### Startup

Sistem boot akışını ve başlangıç modlarını yönetir.

Örnekler:

- Tracking Mode
- Manual Mode
- Calibration

Startup katmanı kaç eksen bulunduğunu bilmek zorunda değildir.
Eksene özel yönlendirme, senkronizasyon ve fan-out işlemleri Axis Manager tarafından yapılır.

### Axis Manager

Startup ile Axis'ler arasındaki orkestrasyon katmanıdır.
Donanım detaylarını bilmez; sadece eksenlerin mantıksal durumunu, hedefini ve senkronizasyonunu yönetir.

Sorumlulukları:

- uygulama komutlarını eksenlere dağıtmak
- pan/tilt gibi çoklu eksen hedeflerini senkronize etmek
- 2 eksenden 6 eksene ölçeklenirken üst katmanları değiştirmemek
- yeni eksen tiplerini aynı arayüz ile yönetmek

Bu katman sayesinde Startup, eksen sayısını ve topolojiyi bilmeden çalışır.

### Services

Sistemin ortak servisleri:

- Logger
- Telemetry
- Configuration
- Diagnostics
- Fault Manager

### Axis

- Pan
- Tilt
- Linear Axis
- Robot Axis

### Control

- Motion Planner
- Controller
- Trajectory

### HAL

- GPIO
- SPI
- UART
- CAN
- Timer
- PWM
- RMT

### Drivers

- Encoder
- Stepper
- IMU
- LED
- Buzzer

## 9. Component Architecture

```text
ACE
|
├── Motion
├── Communication
└── Services
    ├── Planner
    ├── Controller
    ├── Protocol
    ├── Logger
    └── Config
        |
        v
      Axis
        |
      Stepper
        |
      Encoder
```

## 10. Core Software Modules

İlk sürüm aşağıdaki modüllerden oluşacaktır:

- Axis Manager
- Axis
- Motion Planner
- Controller
- Encoder
- Stepper
- HAL
- Drivers
- ALP
- Telemetry
- Logger
- Configuration
- Diagnostics
- Fault Manager
- Health Monitor
- Parameter Manager

## 11. Design Principles

ACE aşağıdaki kurallara göre geliştirilecektir.

### Single Responsibility

Her modül yalnızca tek iş yapacaktır.

### Hardware Independence

Donanım yalnızca Driver katmanında bulunacaktır.

### Layer Isolation

Üst katman alt katmanın iç yapısını bilmeyecektir.

### Message Based Communication

Modüller mümkün olduğunca doğrudan birbirini çağırmak yerine olaylar ve mesajlar üzerinden haberleşecektir.

### Deterministic Execution

Control Loop hiçbir zaman Logger, Telemetry veya UART gibi görevleri beklemeyecektir.

### Safety First

Her hata güvenli duruma geçiş ile sonuçlanacaktır.

## 12. Core Architecture

ACE tamamen Component-Based, Layered ve Event-Driven mimari üzerine kurulacaktır.

Her modül yalnızca kendi sorumluluğunu yerine getirecektir.

### Core View

```text
ACE
|
├── Axis Manager
├── Services
├── Motion
│   ├── Communication
│   ├── Diagnostics
│   └── Config
└── Axis
    ├── Pan Axis
    └── Tilt Axis
        ├── Motion Engine
        └── HAL
            ├── Encoder HAL
            └── Stepper HAL
```

## 13. Axis Framework

ACE'nin en önemli modülü Axis Framework olacaktır.

ACE'de Pan, Tilt, Linear ve Robot Joint aynı sınıf yapısını kullanacaktır.

Örnekler:

- `Axis pan;`
- `Axis tilt;`
- ileride `Axis joint1;`
- ileride `Axis joint2;`
- ileride `Axis joint3;`
- ileride `Axis joint4;`

### Axis Responsibilities

Her Axis yalnızca kendi eksenini bilir:

- Target Position
- Current Position
- Velocity
- Acceleration
- Limits
- Faults
- Calibration
- Controller
- Planner
- Encoder
- Motor

### Internal Architecture

- Motion Planner
- Controller
- Encoder
- Stepper
- Limits
- Calibration
- Diagnostics
- Status

### Public Interface

- `setPosition()`
- `setVelocity()`
- `stop()`
- `hold()`
- `enable()`
- `disable()`
- `home()`
- `calibrate()`
- `update()`
- `getStatus()`

### Design Note: Axis Manager

Startup -> Axis akışı yerine Startup -> Axis Manager -> Axis yapısı kullanılacaktır.

Bu tercih:

- 2 eksenden 6 eksene geçişte üst katmanların değişmesini engeller
- pan ve tilt gibi eksenleri tek merkezden senkronize eder
- robot kolu, lineer eksen ve farklı mekanizmaların eklenmesini kolaylaştırır
- Startup katmanını eksen sayısından bağımsız hale getirir

Bu nedenle Axis Manager, ACE'nin büyüyebilir motion control framework yapısının temel parçasıdır.

Axis Manager sadece mantıksal state ve komut orkestrasyonu yapar.
Motor sürme, encoder okuma ve elektriksel çıkış üretme işi Driver ve HAL katmanına aittir.

## 14. Motion Engine

Motion Engine hareketi üretir.

Hiçbir donanım bilmez, sadece matematik yapar.

### Input

- Current Position

### Output

- Target Velocity

### Motion Engine Bileşenleri

- Planner
- Limiter
- Trajectory
- Controller

## 15. Motion Planner

Görevi "hedefe nasıl gidileceğini" hesaplamaktır.

### Input

- Current Angle
- Target Angle

### Output

- Velocity Profile

### İlk Sürüm

- Trapezoidal Motion

### İkinci Sürüm

- S-Curve Motion

### Garanti Ettiği Limitler

- Max Velocity
- Max Acceleration
- Max Deceleration
- Smooth Stop

## 16. Controller Engine

Controller, Trajectory'den gelen hedefi Encoder'dan gelen gerçek açı ile karşılaştırır.

### Flow

Target -> Error -> Controller -> Velocity

### Sürüm Hedefi

- İlk sürüm: P Controller
- İkinci sürüm: PI
- Üçüncü sürüm: PID
- Dördüncü sürüm: Feed Forward
- Beşinci sürüm: Adaptive Controller

## 17. Position Controller

### Flow

- Target Position
- Current Position
- Position Error
- Velocity Output

## 18. Velocity Controller

### Flow

- Target Velocity
- Current Velocity
- Motor Velocity

## 19. State Machine

Her Axis kendi State Machine'ine sahiptir.

### State Flow

BOOT -> INIT -> DISABLED -> READY -> ENABLED -> IDLE -> POSITION -> VELOCITY -> TRACKING -> HOMING -> CALIBRATION -> FAULT

### State Descriptions

- BOOT: Sistem yeni açıldı.
- INIT: Driver'lar yükleniyor.
- DISABLED: Motor enerjisiz.
- READY: Motor aktif edilebilir.
- ENABLED: Servo hazır.
- POSITION: Açı kontrolü.
- VELOCITY: Hız kontrolü.
- TRACKING: AI sistemi hedef veriyor.
- HOMING: Referans noktası bulunuyor.
- CALIBRATION: Encoder offset hesaplanıyor.
- FAULT: Motor durdurulur.

## 20. Fault Manager

ACE'nin en önemli modüllerinden biridir.

Her hata tek merkezden yönetilir.

### Fault List

- Encoder Fault
- Motor Fault
- Communication Timeout
- Driver Fault
- Motion Timeout
- Emergency Stop
- Over Temperature
- Calibration Error
- Axis Limit
- Configuration Error

### Fault Metadata

Her fault aşağıdaki alanları taşır:

- ID
- Severity
- Timestamp
- Description
- Recovery Method

## 21. Safety Manager

Motion sistemlerinde Safety ayrı modüldür.

### Sorumluluklar

- Soft Limits
- Hard Limits
- Emergency Stop
- Communication Timeout
- Watchdog
- Axis Disable
- Fault Recovery

Safety, Controller'dan bağımsızdır.

## 22. Calibration Manager

Encoder olduğu için şarttır.

### Görevler

- Encoder Offset
- Direction
- Center Position
- Mechanical Limits
- Save Calibration

Kalibrasyon Flash'a kaydedilir.

## 23. Communication Architecture (ALP)

```text
Upper Computer
        |
        v
      ALP
        |
        v
Protocol Parser
        |
        v
Command Dispatcher
        |
        v
Axis Manager
```

### Supported Commands

- Supported command list and ID table live in `PROTOCOL_REFERENCE.md`.
- Protocol commands:
  - `SET_POSITION`
  - `SET_VELOCITY`
  - `STOP`
  - `ENABLE`
  - `DISABLE`
  - `HOME`
  - `CALIBRATE`
  - `GET_TELEMETRY`
  - `HEARTBEAT`

### Telemetry Messages

- `STATUS`
- `POSITION`
- `VELOCITY`
- `FAULT`
- `HEALTH`
- `HEARTBEAT`

## 24. Directory Structure

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
├── CMakeLists.txt
└── sdkconfig.defaults
```

## 25. Coding Standards

ACE aşağıdaki kodlama standartlarını takip edecektir.

- C++17
- No malloc()
- No delay()
- No blocking code
- RAII
- constexpr
- enum class
- std::array
- const correctness
- Thread Safe
- One Class One File
- CamelCase Class
- snake_case variable

## 26. Development Roadmap

### Phase 1

HAL

### Phase 2

Drivers

### Phase 3

Axis

### Phase 4

Motion Planner

### Phase 5

Controller

### Phase 6

ALP

### Phase 7

Telemetry

### Phase 8

AI Tracking

## 27. Acceptance Criteria

ACE aşağıdaki kriterleri karşılamalıdır.

- Encoder okunuyor
- Position Mode çalışıyor
- Velocity Mode çalışıyor
- 1000Hz Loop
- Soft Limit
- Emergency Stop
- ALP çalışıyor
- Telemetry çalışıyor

## 28. Future Roadmap

### v1.0

- Stepper
- Encoder
- UART

### v1.1

- CAN

### v1.2

- IMU

### v1.3

- S-Curve

### v2.0

- Servo

### v3.0

- Ethernet

### v4.0

- Linux Port

## 29. Internal Event Bus

Hiçbir modül için ayrı bir event bus framework yazılmayacaktır.
ACE, bu ihtiyacı FreeRTOS Queue ile karşılayacaktır.

### Sadeleştirilmiş Akış

- Protocol Parser -> FreeRTOS Queue -> Axis Manager -> Axis
- Communication -> FreeRTOS Queue -> Axis Manager -> Axis
- Safety -> FreeRTOS Queue -> Axis Manager -> Axis
- Fault Manager -> FreeRTOS Queue -> Axis Manager -> Axis

Bu yaklaşım:

- ayrı bir event bus katmanı oluşturmaz
- ESP32 üzerinde gereksiz soyutlama yükü üretmez
- mevcut FreeRTOS altyapısını doğrudan kullanır
- yine de modüller arası gevşek bağlılık sağlar

## 30. Message Queue

Tüm olaylar Queue üzerinden aktarılır.

Örnek akış:

- `SET_POSITION`
- Protocol -> Queue -> Axis
- Encoder Fault -> Queue -> Fault Manager -> Safety -> Axis

## 31. Scheduler

ACE FreeRTOS kullanacaktır.

### Task List

- Control Loop
- Communication
- Telemetry
- Planner
- Logger
- Background

### RTOS Task Map

| Module | Type | Frequency / Trigger | Notes |
| --- | --- | --- | --- |
| `app_main()` | Startup | Once | System bootstrap only |
| `HAL` | Startup | Once | Peripheral init |
| `Control Loop` | Periodic | 1000 Hz | Axis orchestration, control and state handling |
| `Axis Manager` | Event-driven + orchestration | Within control loop | Distributes commands to axes |
| `Motion Planner` | Periodic | 500 Hz | Trajectory generation |
| `Communication` | Periodic / interrupt-backed | 100 Hz | Link handling and receive pump |
| `Telemetry` | Periodic | 50 Hz | Status publishing |
| `Background` | Periodic | 20 Hz | Diagnostics, health and safety checks |
| `Logger` | Event-driven / low priority | Background | Non-blocking logging |
| `Safety` | Event-driven + fast fault path | On fault / watchdog | Immediate safe-state handling |
| `Calibration` | Event-driven | On `HOME` / `CALIBRATE` | Homing and calibration routines |
| `Fault Manager` | Event-driven | On fault | Centralized fault handling |

## 32. RTOS Timing

### Periodic Timing

| Task | Frequency |
| --- | --- |
| Control Loop | 1000 Hz |
| Planner | 500 Hz |
| Communication | 100 Hz |
| Telemetry | 50 Hz |
| Logger | Background |
| Background | 20 Hz |

### Execution Rules

- Control path must not wait for logger or telemetry.
- Axis Manager runs inside the control loop and may dispatch commands from queue without blocking it.
- Safety and Fault Manager must be able to preempt normal motion flow.
- Calibration and homing must run outside the 1000 Hz hard control path.

## 33. Inter Module Communication

Hiçbir modül `Axis->Controller->Encoder` şeklinde birbirine bağımlı olmayacak.

Onun yerine:

- Publish
- Queue
- Subscriber

kullanılacak.

## 34. Object Model

```text
ACE
|
├── Axis Manager
│   ├── Pan Axis
│   └── Tilt Axis
├── Services
├── Communication
├── Configuration
├── Logger
├── Diagnostics
├── Safety
├── Calibration
└── HAL
```
