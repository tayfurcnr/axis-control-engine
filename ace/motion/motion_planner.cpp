#include "ace/motion/motion_planner.hpp"
#include <algorithm>
#include <cmath>

namespace ace::motion {

// SUMMARY: DONE: Fiziksel hız ve ivmelenme sınırlarını ayarlar; planlayıcının hareket zarflığını belirler.
void MotionPlanner::set_limits(float max_velocity_dps, float max_acceleration_dps2)
{
    max_velocity_ = std::abs(max_velocity_dps);
    max_acceleration_ = std::abs(max_acceleration_dps2);
}

// SUMMARY: DONE: Ulaşılmak istenen hedef açı konumunu planlayıcıya güncellemek için bildirir.
void MotionPlanner::set_target(float target_deg)
{
    target_position_ = target_deg;
}

// SUMMARY: DONE: Planlayıcıyı belirli bir pozisyona göre sıfırlar; anlık hız ve hedef bilgisini temizler.
void MotionPlanner::reset(float current_position_deg)
{
    current_position_ = current_position_deg;
    target_position_ = current_position_deg;
    current_velocity_ = 0.0f;
}

// SUMMARY: DONE: Trapezoidal hız profilini her zaman adımında (dt) güncelleyerek İdeal setpoint pozisyon ve hızını döndürür.
MotionState MotionPlanner::update(float dt_s)
{
    if (dt_s <= 0.0f) {
        return {current_position_, current_velocity_};
    }

    float diff = target_position_ - current_position_;
    float abs_diff = std::abs(diff);

    // Calculate maximum allowed velocity to not overshoot the target.
    // Derived from v^2 = u^2 + 2as -> v = sqrt(2 * a * distance)
    float safe_velocity = std::sqrt(2.0f * max_acceleration_ * abs_diff);
    
    // Desired ideal velocity limits to our physical cap and safe stopping curve
    float ideal_velocity = std::min(safe_velocity, max_velocity_);
    if (diff < 0.0f) {
        ideal_velocity = -ideal_velocity;
    }

    // Now slew the current velocity to the ideal velocity respecting max acceleration
    float velocity_error = ideal_velocity - current_velocity_;
    float max_delta_velocity = max_acceleration_ * dt_s;

    if (std::abs(velocity_error) <= max_delta_velocity) {
        current_velocity_ = ideal_velocity;
    } else {
        if (velocity_error > 0.0f) {
            current_velocity_ += max_delta_velocity;
        } else {
            current_velocity_ -= max_delta_velocity;
        }
    }

    // Prevent floating point drift causing jitter around zero speed/zero distance
    if (abs_diff < 0.01f && std::abs(current_velocity_) < 0.01f) {
        current_position_ = target_position_;
        current_velocity_ = 0.0f;
    } else {
        current_position_ += current_velocity_ * dt_s;
    }

    return {current_position_, current_velocity_};
}

}  // namespace ace::motion
