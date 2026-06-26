# HAL (Hardware Abstraction Layer) Modülü

`ace/hal`, cihazın PID ve Controller matematik blokları ile Drivers modülündeki saf ESP32 kodları arasında soyutlama (Abstraction) yapar. Motorlar ve Enkoderler bu katmandaki classlara sokularak kodun geri kalanı tarafından sanki basit bir C++ nesnesiymiş gibi temiz bir arayüzle kullanılabilir.
