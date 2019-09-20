#pragma once
#include "config.h"
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>



/* Commands & messages from Thermostat Client
 *  Command  OFF from Thermostat client
 {
  "ID": "1",
  "cmd":"off",
  "actualTEMP":"36.6",
  "targetTEMP":"36.6"
} 
  Command ON from Thermostat Client
  "ID": "1",
  "cmd":"on",
  "actualTEMP":"34.6",
  "targetTEMP":"36.6"

  Message answering from question "show"
   {
  "ID": "1",
  "cmd":"checkin",
  "actualTEMP":"36.6",
  "targetTEMP":"36.6",
  "version":"0.1",
  "serial":"123",
  "IP": "192.168.1.5"
} 
 */

class UDPMessengerService {
  private:
    static const int _MAX_PACKET_SIZE = 512;
    uint16_t _listenPort;
    IPAddress serverIP;
    time_t _currentCommand = -1;
    WiFiUDP _udp;
    void processMessage(IPAddress senderIp, uint16_t senderPort, char *message);
    void sendPacket(IPAddress ip, bool broadcast,uint16_t port, const char* content);
    void getDeviceInfo(JsonObject &result);
    
  public:
    char cServerIP[16];
    char lastcServerIP[16];
    bool serverConnected = false;  
    UDPMessengerService(uint16_t port);
    void listen();
    void registerInServer();
    void searchForServer();
 
    time_t sendTempCommand(float actualTemp, float targetTemp, bool switchOn, int thermoID); //return commandID, server confirm by sending OK + command ID
};
