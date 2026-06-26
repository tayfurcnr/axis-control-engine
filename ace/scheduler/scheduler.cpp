#include "ace/scheduler/scheduler.hpp"

#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace ace::scheduler {

Scheduler* Scheduler::instance_ = nullptr;

namespace {

constexpr TickType_t kControlPeriod = pdMS_TO_TICKS(1);
constexpr TickType_t kPlannerPeriod = pdMS_TO_TICKS(2);
constexpr TickType_t kCommunicationPeriod = pdMS_TO_TICKS(10);
constexpr TickType_t kTelemetryPeriod = pdMS_TO_TICKS(20);
constexpr TickType_t kBackgroundPeriod = pdMS_TO_TICKS(50);

}  // namespace

Scheduler::Scheduler(ace::axis::AxisManager& axis_manager,
                     ace::communication::AlpParser& alp_parser,
                     ace::protocol::ProtocolDispatcher& dispatcher,
                     ace::telemetry::TelemetryPublisher& telemetry,
                     ace::diagnostics::Diagnostics& diagnostics,
                     ace::safety::SafetyManager& safety,
                     ace::services::Logger& logger)
    : axis_manager_(axis_manager),
      alp_parser_(alp_parser),
      dispatcher_(dispatcher),
      telemetry_(telemetry),
      diagnostics_(diagnostics),
      safety_(safety),
      logger_(logger)
{
}

// SUMMARY: DONE: Tüm FreeRTOS görevlerini (task) oluşturarak zamanlama döngülerini başlatır.
void Scheduler::start()
{
    instance_ = this;

    xTaskCreate(control_loop_task, "ace_ctrl_loop", 4096, nullptr, 5, nullptr);
    xTaskCreate(planner_task, "ace_planner", 4096, nullptr, 4, nullptr);
    xTaskCreate(communication_task, "ace_comm", 4096, nullptr, 3, nullptr);
    xTaskCreate(telemetry_task, "ace_telemetry", 4096, nullptr, 2, nullptr);
    xTaskCreate(background_task, "ace_bg", 4096, nullptr, 2, nullptr);

    logger_.info("Scheduler started");
}

// SUMMARY: DONE: 1 ms periyotta (1000 Hz) çalışan ana PID/kontrol döngüsü görevi.
void Scheduler::control_loop_task(void* arg)
{
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        instance_->run_control_loop();
        vTaskDelayUntil(&last_wake, kControlPeriod);
    }
}

// SUMMARY: DONE: 2 ms periyotta (500 Hz) çalışan hareket planlayıcı görevi.
void Scheduler::planner_task(void* arg)
{
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        instance_->run_planner_loop();
        vTaskDelayUntil(&last_wake, kPlannerPeriod);
    }
}

// SUMMARY: DONE: 10 ms periyotta (100 Hz) çalışan ALP protokol ayrıştırıcı ve komut gelen kutusu görevi.
void Scheduler::communication_task(void* arg)
{
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        instance_->run_communication_loop();
        vTaskDelayUntil(&last_wake, kCommunicationPeriod);
    }
}

// SUMMARY: DONE: 20 ms periyotta (50 Hz) çalışan eksen durumünü dışarıya ileten telemetri görevi.
void Scheduler::telemetry_task(void* arg)
{
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        instance_->run_telemetry_loop();
        vTaskDelayUntil(&last_wake, kTelemetryPeriod);
    }
}

// SUMMARY: DONE: Güvenlik aktifse eksenleri devre dışı bırakır, aksi halde 1ms AxisManager::update döngüsünü çalıştırır.
void Scheduler::run_control_loop()
{
    if (safety_.safe_state_active()) {
        axis_manager_.disable(ace::communication::AxisId::ALL);
        return;
    }

    axis_manager_.update(0.001f); // 1 ms period
}

// SUMMARY: TODO: ALP ayrıştırıcının 500Hz döngüsünde yazılımsal ön çalışma sırasını işler; ALP entegrasyonu sonra bağlanacak.
void Scheduler::run_planner_loop()
{
    // Placeholder for the 500 Hz trajectory planner.
    (void)alp_parser_;
}

// SUMMARY: TODO: Gelen ALP komutunu parse edecek ve dispatch edecek; ALP entegrasyonu sonra bağlanacak.
void Scheduler::run_communication_loop()
{
    // Placeholder for the 100 Hz receive and dispatch path.
    (void)dispatcher_;
    if (const auto event = axis_manager_.consume_event()) {
        logger_.debug("event", "Asenkron event tüketildi.");
        (void)event;
    }
}

// SUMMARY: DONE: TelemetryPublisher aracılığıyla eksen durum bilgisini 50 Hz paket oranında dışarı iletir.
void Scheduler::run_telemetry_loop()
{
    telemetry_.publish_status(axis_manager_);
}

// SUMMARY: DONE: 50 ms periyotta çalışan arka plan görev döngüsü (diagnostics, low-priority işlemler).
void Scheduler::background_task(void* arg)
{
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        instance_->run_background_loop();
        vTaskDelayUntil(&last_wake, kBackgroundPeriod);
    }
}

// SUMMARY: DONE: Diagnostics öz-test başarısız olursa sistemi güvenli duruma (SafeState) alır.
void Scheduler::run_background_loop()
{
    if (!diagnostics_.run_self_test()) {
        safety_.enter_safe_state();
        return;
    }

    (void)safety_;
}

}  // namespace ace::scheduler
