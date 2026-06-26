#pragma once

namespace ace::motion {

struct MotionState {
    float position_deg;
    float velocity_dps;
};

class MotionPlanner {
public:
    MotionPlanner() = default;

    // Set physical mechanical limits
    void set_limits(float max_velocity_dps, float max_acceleration_dps2);

    // Apply a new position target
    void set_target(float target_deg);

    // Hard reset the location of the planner, typically called when starting up 
    // or switching back from manual to auto motion control.
    void reset(float current_position_deg);

    // Calculate the next target point in the internal trajectory
    [[nodiscard]] MotionState update(float dt_s);

private:
    float max_velocity_ = 0.0f;
    float max_acceleration_ = 0.0f;

    float current_position_ = 0.0f;
    float current_velocity_ = 0.0f;
    float target_position_ = 0.0f;
};

}  // namespace ace::motion
