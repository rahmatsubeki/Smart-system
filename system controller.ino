#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6jFzsNsnD"
#define BLYNK_TEMPLATE_NAME "monitor"

#ifndef STASSID
#define STASSID "RMT"
#define STAPSK "dawarblandong"
#endif

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//serial jaringan
#include <TelnetStream.h>


#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;
unsigned long previousMillis = 0;
unsigned long interval = 2000;


const int analogpin= A0;
const int relayPin = D8;
const float noVoltageThreshold = 0.1;

//blynk
char auth[] = "a4yGOxotTKYYGbUgrB2vivNfuHGBs3rS";
const char* ssid = STASSID;
const char* password = STAPSK;

float shuntvoltage = 0.00;
float busvoltage = 0.00;
float current = 0.00;
float loadvoltage = 0.00;
float energy = 0.00,  energyCost, energyPrevious, energyDifference;
float energyPrice = 6.50 ;
float power = 0.00;
float capacity = 0.00;

void setup() {
  Serial.begin(115200);
  while(!Serial){
    delay(1);
  }
  
  uint32_t currentFrequency;
  if(!ina219.begin()){
    Serial.println("Sensor Tidak Ditemukan");
    while(1){
      delay(10);
    }
  }
  // put your setup code here, to run once:
  pinMode(relayPin, OUTPUT);
  Blynk.begin(auth, ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
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
  //OTA
  ArduinoOTA.begin();
  TelnetStream.begin();
}

void loop() {
  ArduinoOTA.handle();

  Blynk.run();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    sistemku();
  } 
}
void sistemku(){
//sell surya
  float R1 = 30000.0;
  float R2 = 7500.0;
  float hasil= 0.0;
  int analogValue = analogRead(analogpin);
  float value = analogValue * (3.3/1023.0);
  hasil = (value / (R2 / (R1+R2)));
  if (value < noVoltageThreshold) {
    hasil = 0.0;
  }

  //sytem
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  power = loadvoltage * current;
  energy = energy + power / 3600; //Wh
  capacity = capacity + current / 1000;
  energyDifference = energy - energyPrevious;
  energyPrevious = energy;
  energyCost = energyCost + (energyPrice / 1000 * energyDifference);
  if (loadvoltage < 11) {
    // Jika nilai arus kurang dari 1 A
    digitalWrite(relayPin, LOW);  // Menghidupkan relay
    Blynk.virtualWrite(V5, String("System OFF"));
    Blynk.virtualWrite(V6, 1);
    Serial.println("Nilai arus kurang dari 1 A - Relay ON");
  } else {
    // Jika nilai arus tidak kurang dari 1 A
    digitalWrite(relayPin, HIGH);  // Mematikan relay
    Blynk.virtualWrite(V5, String("System ON"));
    Blynk.virtualWrite(V6, 0);
    Serial.println("Nilai arus lebih dari atau sama dengan 1 A - Relay OFF");
  }
  TelnetStream.println(busvoltage);
  if (loadvoltage < 1 )loadvoltage = 0;
  if (current < 1 )
  {
    current = 0;
    power = 0;
    energy = 0;
    capacity = 0;
    energyCost=0;
  }
  Serial.print("Voltage:   "); Serial.print(value); Serial.println(" V");
  Serial.print("Voltage pengukuran:   "); Serial.print(hasil); Serial.println(" V");
  Serial.print("hasil bacaan:   "); Serial.println(analogValue); 
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power); Serial.println(" mW");
  Serial.print("Energy:        "); Serial.print(energy); Serial.println(" Wh");
  Serial.println("-------------------------");

  // put your main code here, to run repeatedly:
  Blynk.virtualWrite(V0, hasil);//plts
  Blynk.virtualWrite(V1, (current / 1000) );//amper pengguna
  Blynk.virtualWrite(V2, (power / 1000) );//watt pengguna
  Blynk.virtualWrite(V3, (energy / 1000));// konsumsi
  Blynk.virtualWrite(V7, loadvoltage);//voltase pengguna
  // CAPACITY
  if (capacity > 1000) {
    Blynk.virtualWrite(V4, String((capacity / 1000), 2) + String(" Ah") );
  }
  else
  {
    Blynk.virtualWrite(V4, String((capacity), 2) + String(" mAh") );
  }
  
}
