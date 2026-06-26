# Motion Modülü

`ace/motion` paketi eksenlerin hız rampası ve ani hareketleri kısıtlayıcı fizik yapılarını yönetir.

### Motion Planner (Trapezoidal Slew-Rate Limiting)
Basit bir PID komutu doğrudan hedefe yönelmeye çalıştığında ani sarsıntılara neden olur. `MotionPlanner` sınıfı, "belirlenmiş bir varış hedefine" yönelmeden önce belirli bir fiziksel mantıkta o hedefi parçalara böler.

- Tanımlanmış olan **Maksimum İvmelenme** (`max_acceleration_dps2`) ve **Maksimum Hız** (`max_velocity_dps`) fizik sınırlarını izler.
- Durma mesafesini (Stopping distance) dinamik olarak hesaplayıp hedefe yaklaşınca frenlemeyi (yavaşlama rampasını) başlatır.
- Çözünürlüğü `dt` süre bazlıdır ve AxisManager içerisinden PID'ye beslenen 'İdeal / Güvenli setpoint konumunu' yayar.
