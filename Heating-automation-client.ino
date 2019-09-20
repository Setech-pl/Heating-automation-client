#include "config.h"
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "udpmessengerservice.h"
#include "secrets.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
UDPMessengerService udpMessenger(_HEATING_PORT);
unsigned long timeMillis = 0;
unsigned long commandMillis = 0;
float actualTemp = 20;
float targetTemp = 21;
float actualHum = 81.2;

void setup()
{
  Serial.begin(19200);
  Serial.println("Heating client");
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting WiFi");
  int counter = 0;
  WiFi.disconnect();
  WiFi.begin(_EXTERNAL_WIFI_SID, _EXTERNAL_WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED and counter < 10)
  {
    counter++;
    delay(500);
  }
  lcd.clear();
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.setCursor(0, 0);
    lcd.print("WiFi connected  ");
    char hr[21];
    sprintf(hr, "IP :%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    lcd.setCursor(0, 1);
    lcd.print(hr);
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("Ext Wifi error    ");
    delay(2000);
    ESP.restart();
  }
  delay(5000);
  lcd.clear();

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.println("Updating " + type);
    delay(50);
  });
  ArduinoOTA.onEnd([]() {
    lcd.clear();
    lcd.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    lcd.setCursor(0, 1);
    lcd.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    lcd.clear();
    if (error == OTA_AUTH_ERROR)
    {
      lcd.println("Auth error");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      lcd.println("OTA begin err");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      lcd.println("OTA connect err");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      lcd.println("OTA receive err");
    }
    else if (error == OTA_END_ERROR)
    {
      lcd.println("OTA end error");
    }
  });
  ArduinoOTA.begin();
}

void loop()
{
  if (timeMillis + 5000 < millis())
  {
    timeMillis = millis();
    //read temperature from sensor

    /*

    */

    if (!udpMessenger.serverConnected)
    {
      lcd.setCursor(0, 0);
      lcd.println("Looking for     ");
      lcd.setCursor(0, 1);
      lcd.println("Heating server..");
      udpMessenger.searchForServer();
    }
    else
    {
      // i am connected to server
      if (strcmp(udpMessenger.lastcServerIP, udpMessenger.cServerIP) != 0)
      {
        strcpy(udpMessenger.lastcServerIP, udpMessenger.cServerIP);
        lcd.setCursor(0, 0);
        lcd.println("Connected to  ");
        lcd.setCursor(0, 1);
        lcd.println(udpMessenger.cServerIP);
        lcd.print("  ");
      }
      else
      {
        char hr[21];
        lcd.setCursor(0, 0);
        sprintf(hr, "%2.1f    act:%2.1f", targetTemp, actualTemp);
        lcd.print(hr);
        lcd.setCursor(0, 1);
        sprintf(hr, "%02d:%02d   hum:%2.1f", hour(), minute(), actualHum);
        lcd.print(hr);
      }
      if (commandMillis + _COMMAND_INTERVAL < millis())
      {
        commandMillis = millis();
      }
    }
  }
  udpMessenger.listen();
  ArduinoOTA.handle();
  //read input keys

  //** increase targetTemp
}
