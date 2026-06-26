#pragma once

namespace ace::control {

class Controller {
public:
    Controller() = default;

    void set_gains(float kp, float ki, float kd, float feed_forward = 0.0f);
    void set_limits(float max_integral, float max_output);
    void reset();

    // Computes control effort. 
    // Returns a value clamped to [-max_output, max_output].
    [[nodiscard]] float update(float setpoint, float current_value, float dt_s);

private:
    float kp_ = 0.0f;
    float ki_ = 0.0f;
    float kd_ = 0.0f;
    float feed_forward_ = 0.0f;

    float max_integral_ = 0.0f;
    float max_output_ = 0.0f;

    float integral_ = 0.0f;
    float prev_error_ = 0.0f;
};

}  // namespace ace::control
