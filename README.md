# Monitoring Baterai PLTS dengan ESP8266

Proyek ini memungkinkan Anda untuk memonitor tegangan baterai PLTS dengan menggunakan ESP8266, sensor tegangan analog, sensor INA219, relay 5 volt, LED 12 Watt sistem DC, dan LED 3 Watt sistem DC.

## Komponen yang Dibutuhkan
- ESP8266 (NodeMCU, Wemos, atau modul ESP8266 lainnya)
- Sensor Tegangan Analog
- Sensor INA219
- 4 Relay 5 Volt
- LED 12 Watt Sistem DC
- LED 3 Watt Sistem DC
- Baterai PLTS
- Breadboard dan Kabel Penghubung

## Cara Menggunakan Proyek Ini
1. **Hubungkan Hardware:**
   - Hubungkan sensor tegangan analog ke ESP8266 untuk mengukur tegangan baterai PLTS.
   - Sambungkan sensor INA219 untuk mengukur arus baterai.
   - Hubungkan relay 5 volt dengan ESP8266 untuk mengendalikan daya LED 12 Watt dan LED 3 Watt.
   - Sambungkan LED 12 Watt dan LED 3 Watt dengan relay untuk mengendalikan pencahayaan.

2. **Unggah Kode:**
   - Buka Arduino IDE dan unggah kode monitoring ke ESP8266.
   - Pastikan mengonfigurasi SSID dan kata sandi WiFi di kode.

3. **Jalankan Aplikasi:**
   - Nyalakan ESP8266.
   - Aplikasi akan mulai memonitor tegangan dan arus baterai PLTS.
   - Relay akan mengontrol daya pencahayaan berdasarkan data yang diperoleh dari sensor.

## Kontribusi
Jika Anda ingin berkontribusi pada proyek ini atau melaporkan masalah, silakan buka *Issue* atau kirim *Pull Request*.

## Lisensi
Proyek ini dilisensikan di bawah [MIT License](LICENSE).
