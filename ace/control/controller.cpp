#include "ace/control/controller.hpp"
#include <algorithm>
#include <cmath>

namespace ace::control {

// SUMMARY: DONE: PID kazançlarını (Kp, Ki, Kd) ve ileri besleme (Feed-Forward) ağırlığını günceller.
void Controller::set_gains(float kp, float ki, float kd, float feed_forward)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    feed_forward_ = feed_forward;
}

// SUMMARY: DONE: İntegral büyümesi ve çıkış gorüleri için maksimum sınırları ayarlar.
void Controller::set_limits(float max_integral, float max_output)
{
    max_integral_ = std::abs(max_integral);
    max_output_ = std::abs(max_output);
}

// SUMMARY: DONE: İntegral ve önceki hata değerlerini sıfırlayarak kontrolörü temiz duruma alır.
void Controller::reset()
{
    integral_ = 0.0f;
    prev_error_ = 0.0f;
}

// SUMMARY: DONE: Kapalı döngü PID hesaplamasını yapar; setpoint ile ölçüm farkından motor eforunu (effort) üretir.
float Controller::update(float setpoint, float current_value, float dt_s)
{
    if (dt_s <= 0.0f) {
        return 0.0f;
    }

    float error = setpoint - current_value;

    integral_ += error * dt_s;
    if (max_integral_ > 0.0f) {
        integral_ = std::clamp(integral_, -max_integral_, max_integral_);
    }

    float derivative = (error - prev_error_) / dt_s;
    
    float output = (kp_ * error) + (ki_ * integral_) + (kd_ * derivative) + (feed_forward_ * setpoint);
    
    if (max_output_ > 0.0f) {
        output = std::clamp(output, -max_output_, max_output_);
    }

    prev_error_ = error;

    return output;
}

}  // namespace ace::control
