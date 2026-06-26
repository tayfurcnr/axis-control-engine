# Protocol Modülü

Uygulama Katmanı Protokolü (ALP) ayrıştırıcısıdır. Dışarıdan UART/Network üzerinden String (metin) dizi olarak gelen komutları (örn: `SET_PID ALL ...`) güvenli şekilde parse eder ve yapı parçalarına ayırarak `communication` modülündeki `CommandRequest` taşıyıcı nesnelerine çevirir. `ProtocolDispatcher` sınıfı çekirdek ile iletişim ağının köprüsüdür.
