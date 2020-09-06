#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "DHT.h"
#include "thermistor.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
DHT dht(4, DHT22);
thermistor therm(0, 10000, 3950);
WiFiUDP ntpUDP;
int hrs;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3 * 3600, 60000);
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\r* * * ESP BOOT * * *"));
  Serial.println(F("WiFi begin!"));
  WiFi.mode(WIFI_STA);
  WiFi.begin("liki", "06086080");
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  digitalWrite(14, 0);
  timeClient.begin();
  
  
dht.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("\n\rWiFi connected!"));
}

void getpr24h() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String lol = "https://ladurbo.tk/esp8266/esp.php?t=";
  lol += String(t);
  lol += "&&h=";
  lol += String(h);
  Serial.print("Температура ");
  Serial.print(t);
  Serial.println();
  Serial.print("Влажность ");
  Serial.print(h);
  Serial.println();
  Serial.println(lol);
  if (https.begin(*client, lol)) {  // HTTPS
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        Serial.println(String("[HTTPS] Received payload: ") + payload);
        int fan = payload.substring(1, payload.indexOf("l", 1)).toInt();
        int light = payload.substring(payload.indexOf("l", 1)+1, payload.length()-1).toInt();
        if (hrs > 23 or hrs < 9){
          Serial.println("Время спать");
          fan = 0;
          light = 0;
        }
        Serial.print("Вентилятор: ");
        Serial.print(fan);
        Serial.print(" Свет: ");
        Serial.print(light);
        Serial.println();
        analogWrite(12, fan);
        analogWrite(13, light);
        //int value = map(therm.getTempAverage(), 11, 21, 350, 1023);
        if (therm.getTempAverage() > 16) {
          digitalWrite(14, 1);
        }
        if (therm.getTempAverage() < 11 or light == 1) {
          digitalWrite(14, 0);
        }
        //Serial.println(value);
        //analogWrite(14, value);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n\r");
  }
}

void gettime(){
  timeClient.update();
  hrs = timeClient.getHours();
  Serial.print("Время: ");
  Serial.print(timeClient.getFormattedTime());
  Serial.println();
}

void loop() {
  gettime();
  getpr24h();
  Serial.print("Термистор ");
  Serial.print(therm.getTempAverage());
  Serial.print(" *C");
  Serial.println();
  Serial.println("Wait 1s before next round to not get banned on API server...");
  delay(1000);
}
