#pragma once

namespace ace::control {

// SUMMARY: DONE: Tamamlayıcı (Complementary) Filtre; jiroskop/step gibi gürültüsüz ama zamanla kayan "oran (rate)" verilerini, pusula/ivmeölçer gibi anlık gürültülü ama mutlak (absolute) referans sağlayan verilerle harmanlar.
class ComplementaryFilter {
public:
    // alpha: Yüksek frekanslı veriye (gyro/step oranı) ne kadar güvenileceği (0.0 - 1.0).
    // Örn: 0.98 -> %98 gyro'ya, %2 pusulaya güven.
    explicit ComplementaryFilter(float alpha = 0.98f);

    void set_alpha(float alpha);
    float get_alpha() const;

    // Filtrenin ilk başlangıç (Homing/Wakeup) değerini 
    // mutlak sensör (Compass/Accel) ile manuel ezmek için kullanılır.
    void reset(float initial_angle_deg);

    // Ana füzyon döngüsü (Her ms çağrılır)
    // rate_dps: Saniyedeki açısal hız (Jiroskoptan veya Step motor komut hızından gelebilir)
    // absolute_deg: Mutlak açı ölçümü (Pusuladan veya İvmeölçerden)
    // dt: Geçen zaman (saniye), örn: 0.001
    // Return: Düzeltilmiş/Harmanlanmış güncel açı (Derece)
    float update(float rate_dps, float absolute_deg, float dt);

    float get_current_estimate() const;

private:
    float alpha_;
    float current_estimate_deg_;
    bool is_initialized_;
};

}  // namespace ace::control
