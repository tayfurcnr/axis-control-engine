#include "ace/axis/axis_manager.hpp"
#include "ace/communication/alp.hpp"
#include "ace/config/config.hpp"
#include "ace/config/persistent_storage.hpp"
#include "ace/diagnostics/diagnostics.hpp"
#include "ace/hal/hal.hpp"
#include "ace/protocol/protocol_dispatcher.hpp"
#include "ace/safety/safety_manager.hpp"
#include "ace/scheduler/scheduler.hpp"
#include "ace/services/logger.hpp"
#include "ace/telemetry/telemetry_publisher.hpp"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void)
{
    // One-time hardware and system bootstrap.
    ace::hal::initialize();

    ace::config::RuntimeConfig runtime_config {};
    runtime_config.debug_enabled = true;

    ace::services::Logger logger;
    logger.set_debug_enabled(runtime_config.debug_enabled);
    logger.debug("boot", "Boot başlatıldı.");

    // NVS is used for persistent configuration such as location and calibration state.
    if (nvs_flash_init() != ESP_OK) {
        logger.debug("nvs", "NVS init başarısız, erase ile yeniden deneniyor.");
        nvs_flash_erase();
        nvs_flash_init();
    }

    ace::config::PersistentStorage persistent_storage;
    ace::config::PersistentConfig persistent_config {};
    if (persistent_storage.load(persistent_config) != ace::config::StorageResult::ok) {
        logger.debug("config", "Persistent config yüklenemedi, varsayılan değerler uygulanıyor.");
        if (persistent_storage.save(persistent_config) != ace::config::StorageResult::ok) {
            logger.debug("config", "Persistent config varsayılanları kaydedilemedi.");
        }
    } else {
        logger.debug("config", "Persistent config başarıyla yüklendi.");
    }

    // Runtime objects live here; Scheduler only keeps references.
    ace::diagnostics::Diagnostics diagnostics;
    ace::safety::SafetyManager safety;
    logger.info("System start");
    logger.debug("boot", "Debug modu aktif.");
    logger.debug("config", persistent_config.has_location ? "Kalıcı konum bilgisi yüklendi." : "Kalıcı konum bilgisi bulunamadı.");
    logger.debug("config", "Başlangıç mod ve konfigürasyon uygulaması yapılıyor.");

    ace::axis::AxisManager axis_manager(&logger);
    axis_manager.bind_persistent_state(&persistent_storage, &persistent_config);
    axis_manager.bind_safety_manager(&safety);
    ace::communication::AlpParser alp_parser;
    ace::protocol::ProtocolDispatcher dispatcher;
    ace::telemetry::TelemetryPublisher telemetry;

    if (persistent_config.has_location) {
        axis_manager.restore_location(persistent_config.location_longitude_deg,
                                      persistent_config.location_latitude_deg,
                                      persistent_config.location_altitude_m);
    }
    axis_manager.set_mode(persistent_config.startup_mode);
    logger.debug("axis", "AxisManager başlangıç konfigürasyonu tamamlandı.");

    ace::scheduler::Scheduler scheduler(axis_manager,
                                        alp_parser,
                                        dispatcher,
                                        telemetry,
                                        diagnostics,
                                        safety,
                                        logger);

    // Abort startup if the system cannot pass self-test.
    if (!diagnostics.run_self_test()) {
        safety.enter_safe_state();
        return;
    }

    // Hand off execution to the RTOS scheduler and leave app_main.
    scheduler.start();
    vTaskDelete(nullptr);
}
