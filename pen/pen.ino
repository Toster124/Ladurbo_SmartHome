#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); 
Adafruit_BMP280 bmp;
String message;
int switch1;
void setup() {
  Wire.begin(0, 2);
  pinMode(3, OUTPUT);
  //Serial.begin(115200);
  bmp.begin();
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.cp437(true);
  
  Serial.println(F("\n\r* * * ESP BOOT * * *"));
  Serial.println(F("WiFi begin!"));
  WiFi.mode(WIFI_STA);
  WiFi.begin("iPhone (Ivan)", "toster124");
  display.setCursor(0, 0); 
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.clearDisplay();
    display.setCursor(0, 0); 
  display.print("Connection to WIFI");
  display.display();
  }
  display.clearDisplay();
  display.setCursor(0, 0); 
  display.print("WIFI connected");
  display.display();
}


void loop() {
  web_request();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(message);
  display.display();
  digitalWrite(3, switch1);
delay(1000);
}

void web_request() {
  float p = bmp.readPressure()/133.3;
  float t = bmp.readTemperature();
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String lol = "https://ladurbo.tk/esp8266/esp2.php?t=";
  lol += String(t);
  lol += "&&p=";
  lol += String(p);
  Serial.println(lol);
  if (https.begin(*client, lol)) {  // HTTPS
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
    }
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        message = payload.substring(0, payload.length()-1);
        switch1 = payload.substring(payload.length()-1, payload.length()).toInt();
        Serial.println(String("[HTTPS] Received payload: ") + payload);
        
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n\r");
  }
}
