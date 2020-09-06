#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "GyverTimer.h" // подключаем библиотеку
int timing = 30, delaytime, pushin, nightfrom = 23, nightto = 5, hrs = 13, sleepcount;
uint32_t turnTimer = 0;
boolean quickpush = 0, longsleep;
GTimer_ms myTimer(20000);
void setup()
{
    pinMode(2, OUTPUT);
    Serial.begin(115200);
    Serial.println("работа полнейшая идёт");
    Serial.println(F("\n\r* * * ESP BOOT * * *"));
    myTimer.setMode(MANUAL);
}

void loop()
{
    if (millis() - turnTimer >= (timing * 60 * 1000) || quickpush)
    {
        turnTimer = millis();
        push();
        quickpush = 0;
    }
    wifiInit();
    if (WiFi.status() == WL_CONNECTED)
    {
        longsleep = 0;
        web_request();
        pushin = ((timing * 60 * 1000) - (millis() - turnTimer)) / 1000;
        if (quickpush == 1) {
          pushin = 0;
        }
        web_request();
    }
    Serial.println("Нажму через " + String(pushin) + "c");
    if (hrs >= nightfrom || hrs < nightto)
    {
        Serial.println("Долгий сон");
        longsleep = 1;
        web_request();
        sleepcount = (24 - nightfrom) + nightto;
        lightSleep(sleepcount * 60 * 60 * 1000);
    }
    else
    {
        Serial.println("СПААААААААТЬ");
        goToLightSleep();
    }
}
void web_request()
{
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
    String lol = "https://ladurbo.tk/esp8266/esp3.php?p=";
    lol += String(pushin);
    lol += "&&s=";
    lol += String(longsleep);
    // Serial.println(lol);
    if (https.begin(*client, lol))
    { // HTTPS
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        }
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = https.getString();
            timing = payload.substring(1, payload.indexOf("nf")).toInt();
            Serial.println(timing);
            nightfrom = payload.substring(payload.indexOf("nf") + 2, payload.indexOf("nt")).toInt();
            Serial.println(nightfrom);
            nightto = payload.substring(payload.indexOf("nt") + 2, payload.indexOf("hr")).toInt();
            Serial.println(nightto);
            hrs = payload.substring(payload.indexOf("hr") + 2, payload.length() - 1).toInt();
            Serial.println(hrs);
            // Serial.println(String("[HTTPS] Received payload: ") + payload);
        }
        else
        {
            Serial.printf(
                "[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());
        }
        https.end();
    }
    else
    {
        Serial.printf("[HTTPS] Unable to connect\n\r");
    }
}

void push()
{
    Serial.println("Нажимаю");
    digitalWrite(2, 1);
    delay(1000);
    digitalWrite(2, 0);
}

void goToLightSleep()
{
    if ((timing * 60 * 1000) < 300000)
    {
        delaytime = timing * 60 * 1000;
        quickpush = 1;
    }
    else
    {
        delaytime = 300000;
    }
    lightSleep(delaytime);
}

void lightSleep(int timeinsleep)
{
    Serial.println("Заснул на " + String(timeinsleep/1000) + "с");
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.forceSleepBegin();
    delay(3);
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(0xFFFFFFF);
    delay(timeinsleep);
    WiFi.forceSleepWake();
    Serial.println("Woke up!");
}

void wifiInit()
{
    Serial.println(F("WiFi begin!"));
    WiFi.mode(WIFI_STA);
    WiFi.begin("liki", "06086080");
    myTimer.reset();
    myTimer.start();
    while (WiFi.status() != WL_CONNECTED && !myTimer.isReady())
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
}
