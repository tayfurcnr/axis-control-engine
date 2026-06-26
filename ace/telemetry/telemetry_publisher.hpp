#pragma once

#include "ace/axis/axis_manager.hpp"

namespace ace::telemetry {

class TelemetryPublisher {
public:
    void publish_status(const ace::axis::AxisManager& axis_manager) const;
};

}  // namespace ace::telemetry
