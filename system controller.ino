#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6L8dR8HBQ"
#define BLYNK_TEMPLATE_NAME "System Plts"
#define BLYNK_FIRMWARE_VERSION "1.0"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
WiFiClient client;

#define UPDATE_BUTTON_PIN V9

// WiFi credentials dan token Blynk
char auth[] = "Zu7zjfvslnsdPjd0F4HMfIG5CL00_vOw";
const char* ssid = "RMT";
const char* password = "dawarblandong";
const char* firmwareURL = "https://github.com/rahmatsubeki/Smart-system/blob/main/system%20controller.ino";  // Ganti URL dengan URL firmware di GitHub
const int currentFirmwareVersion = 1; 

// Sensor INA219
Adafruit_INA219 ina219;

// Pin Relay
const int relayPin1 = D5;
const int relayPin2 = D6;
const int relayPin3 = D7;
const int relayPin4 = D8;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//progamina
const float noVoltageThreshold = 0.1;
float shuntvoltage = 0.00;
float busvoltage = 0.00;
float loadvoltage = 0.00;
float current = 0.00;
float power = 0.00;
float energy = 0.00,  energyCost, energyPrevious, energyDifference;
float capacity = 0.00;
float avgBusVoltage = 0.0;
const int numReadings = 10;
float calculateAverageBusVoltage() {
  float totalBusVoltage = 0.0;

  for (int i = 0; i < numReadings; i++) {
    // Membaca nilai tegangan bus
    float busVoltage = ina219.getBusVoltage_V();
    totalBusVoltage += busVoltage;

    // Menunggu sebelum membaca nilai berikutnya
    delay(1000); // Tunggu 1 detik (1000 ms)
  }

  // Menghitung rata-rata tegangan bus selama periode waktu
  float averageBusVoltage = totalBusVoltage / numReadings;
  return averageBusVoltage;
}

BLYNK_WRITE(V9) {  // Fungsi ini akan dipanggil saat tombol di Blynk ditekan
  int buttonState = param.asInt();
  if (buttonState == HIGH) {  // Tombol ditekan
    Blynk.virtualWrite(V10,"Memeriksa Pembaruan...");
    delay(10000);
    checkUpdateButton();
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  ina219.begin();
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);

  // Synchronize waktu dengan server NTP
  configTime(7 * 3600, 0, "pool.ntp.org");

  // Tunggu sampai terhubung ke WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set waktu yang benar setelah mendapatkan koneksi WiFi
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.println("Time synchronized");

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.setHostname("Batrei_Monitoring");
  ArduinoOTA.begin();
}

void loop() {
  Blynk.run();
  timeClient.update();
  ArduinoOTA.handle();
  // Dapatkan waktu saat ini
  int currentHour = (timeClient.getHours() + 7)%24;
  int currentMinute = timeClient.getMinutes();
  
  // Mengaktifkan relay pada pukul 17:30 dan mematikannya pada pukul 05:30
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  power = loadvoltage * current;
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current = ina219.getCurrent_mA();
  energy = energy + power / 3600 / 1000; //kwh
  //deklarasi berdasarkan
  if (loadvoltage < 1 )loadvoltage = 0;
  if (current < 1 )
  {
    current = 0;
    power = 0;
    energy = 0;
    capacity = 0;
    energyCost=0;
  }

  String systemStatusMessage;

  if(loadvoltage > 11.0){
      digitalWrite(relayPin1, HIGH);
      systemStatusMessage = "System ON";
      // digitalWrite(relayPin2, HIGH);

      if ((currentHour == 4 && currentMinute >= 15) || (currentHour >= 4 && currentHour <= 16) || (currentHour == 16 && currentMinute <= 30)) {
          avgBusVoltage = calculateAverageBusVoltage();
          digitalWrite(relayPin2, LOW);
      } else {
        if (loadvoltage > 11.4){
          avgBusVoltage = 0.0;
          digitalWrite(relayPin2, HIGH);
        }else{
          avgBusVoltage = 0.0;
          digitalWrite(relayPin2, LOW);
        }
      }
  }else{
      digitalWrite(relayPin1, LOW);
      digitalWrite(relayPin2, LOW);
      systemStatusMessage = "System OFF";
  }


  // Kirim data ke Blynk
  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, avgBusVoltage);
    Blynk.virtualWrite(V1, loadvoltage);//voltase pengguna
    Blynk.virtualWrite(V2, (current / 1000));//amper pengguna
    Blynk.virtualWrite(V3, (power / 1000));//watt pengguna
    Blynk.virtualWrite(V4, energy);// konsumsi
    Blynk.virtualWrite(V5, systemStatusMessage);//status
  }

  // Tunggu 1 detik sebelum membaca sensor dan mengirim data lagi
  delay(5000);
}

void checkUpdateButton() {
  int updateButtonState = digitalRead(UPDATE_BUTTON_PIN);

  // Jika tombol update ditekan
  if (updateButtonState == LOW) {
    Serial.println("Tombol update ditekan!");
    
    // Mengupdate kode dari URL firmware yang ditentukan
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://github.com/rahmatsubeki/Smart-system/blob/main/system%20controller.ino");

    // Memeriksa hasil pembaruan
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Blynk.virtualWrite(V10,"Update gagal");
        Serial.printf("Update gagal, error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;
        
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("Tidak ada pembaruan tersedia.");
        Blynk.virtualWrite(V10,"Tidak ada pembaruan tersedia.");
        break;
        
      case HTTP_UPDATE_OK:
        Blynk.virtualWrite(V10,"Pembaruan berhasil!");
        Serial.println("Pembaruan berhasil!");
        break;
    }

    Blynk.virtualWrite(V9, 0);
  }
}

