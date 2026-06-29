#include <cassert>
#include <cmath>
#include <iostream>

#include "ace/control/sensor_fusion.hpp"

void test_filter_initialization()
{
    ace::control::ComplementaryFilter filter(0.9f);
    
    // Ilk update'te rate onemsiz, %100 absolute degere(PUSULA) kitlenmeli.
    float result = filter.update(100.0f, 45.0f, 0.01f);
    assert(std::fabs(result - 45.0f) < 0.001f);
    assert(std::fabs(filter.get_current_estimate() - 45.0f) < 0.001f);
    std::cout << "test_filter_initialization passed.\n";
}

void test_filter_fusion_logic()
{
    ace::control::ComplementaryFilter filter(0.9f); // %90 Gyro, %10 Compass
    
    // Başlangıç: 0 Derece
    filter.reset(0.0f);
    
    // Senaryo: Cihaz saniyede 10 derece hızla (rate_dps=10) dönüyor.
    // DT = 0.1 saniye (Yani yüksek frekanslı tahmin = 0 + (10 * 0.1) = 1.0 derece olmalı)
    // Ancak Pusula/Manyetik Parazit o an açıyı 10.0 derece (gürültülü/hatalı) okudu.
    
    // Formül: (0.9 * 1.0) + (0.1 * 10.0)
    //       : 0.9 + 1.0 = 1.9 derece
    
    float result = filter.update(10.0f, 10.0f, 0.1f);
    assert(std::fabs(result - 1.9f) < 0.001f);
    std::cout << "test_filter_fusion_logic passed.\n";
}

void test_drift_correction()
{
    ace::control::ComplementaryFilter filter(0.99f); // %99 Gyro (Çok güveniyor)
    filter.reset(0.0f);
    
    // Diyelim ki alet duruyor (rate_dps = 0), pusula da hep 5 derece gosteriyor
    // Zamanla filtre yavas yavas 5 dereceye cekilmeli (Kaymayi/Drifti duzeltmeli)
    for (int i = 0; i < 500; ++i) {
        filter.update(0.0f, 5.0f, 0.01f);
    }
    
    float final_pos = filter.get_current_estimate();
    // 500 iterasyon sonunda aci 0'dan 5'e dogru cekilmis olmali.
    assert(final_pos > 4.9f && final_pos <= 5.0f);
    std::cout << "test_drift_correction passed.\n";
}

int main()
{
    std::cout << "Running SensorFusion Tests...\n";
    test_filter_initialization();
    test_filter_fusion_logic();
    test_drift_correction();
    std::cout << "All SensorFusion Tests passed!\n";
    return 0;
}
