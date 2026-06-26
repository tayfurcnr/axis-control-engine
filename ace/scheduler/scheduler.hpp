#pragma once

#include "ace/axis/axis_manager.hpp"
#include "ace/communication/alp.hpp"
#include "ace/diagnostics/diagnostics.hpp"
#include "ace/protocol/protocol_dispatcher.hpp"
#include "ace/safety/safety_manager.hpp"
#include "ace/services/logger.hpp"
#include "ace/telemetry/telemetry_publisher.hpp"

namespace ace::scheduler {

class Scheduler {
public:
    Scheduler(ace::axis::AxisManager& axis_manager,
              ace::communication::AlpParser& alp_parser,
              ace::protocol::ProtocolDispatcher& dispatcher,
              ace::telemetry::TelemetryPublisher& telemetry,
              ace::diagnostics::Diagnostics& diagnostics,
              ace::safety::SafetyManager& safety,
              ace::services::Logger& logger);

    void start();

private:
    static void control_loop_task(void* arg);
    static void planner_task(void* arg);
    static void communication_task(void* arg);
    static void telemetry_task(void* arg);
    static void background_task(void* arg);

    void run_control_loop();
    void run_planner_loop();
    void run_communication_loop();
    void run_telemetry_loop();
    void run_background_loop();

    ace::axis::AxisManager& axis_manager_;
    ace::communication::AlpParser& alp_parser_;
    ace::protocol::ProtocolDispatcher& dispatcher_;
    ace::telemetry::TelemetryPublisher& telemetry_;
    ace::diagnostics::Diagnostics& diagnostics_;
    ace::safety::SafetyManager& safety_;
    ace::services::Logger& logger_;

    static Scheduler* instance_;
};

}  // namespace ace::scheduler
