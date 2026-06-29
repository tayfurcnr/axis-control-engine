#include "ace/control/sensor_fusion.hpp"
#include <algorithm>

namespace ace::control {

ComplementaryFilter::ComplementaryFilter(float alpha)
    : current_estimate_deg_(0.0f)
    , is_initialized_(false)
{
    set_alpha(alpha);
}

void ComplementaryFilter::set_alpha(float alpha)
{
    alpha_ = std::clamp(alpha, 0.0f, 1.0f);
}

float ComplementaryFilter::get_alpha() const
{
    return alpha_;
}

void ComplementaryFilter::reset(float initial_angle_deg)
{
    current_estimate_deg_ = initial_angle_deg;
    is_initialized_ = true;
}

float ComplementaryFilter::update(float rate_dps, float absolute_deg, float dt)
{
    if (!is_initialized_) {
        // Cihaz yeni açıldıysa, ilk ölçüm %100 mutlak referanstan alınır (Homing).
        reset(absolute_deg);
        return current_estimate_deg_;
    }

    // Açı = (Güven x Yüksek Frekans Tahmini) + (Kalan Güven x Mutlak Referans)
    // Yüksek Frekans Tahmini = (Önceki Açı) + (Hız * dt)
    const float high_freq_estimate = current_estimate_deg_ + (rate_dps * dt);
    
    current_estimate_deg_ = (alpha_ * high_freq_estimate) + ((1.0f - alpha_) * absolute_deg);

    return current_estimate_deg_;
}

float ComplementaryFilter::get_current_estimate() const
{
    return current_estimate_deg_;
}

}  // namespace ace::control
