#pragma  once
#include "config.h"
#include "udpmessengerservice.h"
#include <ArduinoJson.h>
#include <TimeLib.h>

/*
 * Server send cmd SHOW
 * ALL clients sends cmd HELLO
 * Server register clients
 */

UDPMessengerService::UDPMessengerService(uint16_t port) {
  _udp.begin(port);
  _listenPort = port;
}


void UDPMessengerService::listen() {
  int packetSize = _udp.parsePacket();
  if(packetSize) {
    Serial.println("incoming packet");
    char incomingPacket[_MAX_PACKET_SIZE];
    int len = _udp.read(incomingPacket, _MAX_PACKET_SIZE);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    processMessage(_udp.remoteIP(), _udp.remotePort(), incomingPacket);
  }
}

void UDPMessengerService::getDeviceInfo(JsonObject &result) {
  result["serialNumber"] = ESP.getChipId();
  // Mo�emy tutaj dopisa� inne parametry, np. nazw� sieci do kt�rej ESP si� ��czy
}

void UDPMessengerService::sendPacket(IPAddress ip, bool broadcast,uint16_t port, const char* content) {
  //_udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
  if (!broadcast){
    _udp.beginPacket(ip, port);  
  }else {
    _udp.beginPacketMulticast(ip,port,WiFi.localIP());
  }
  _udp.write(content);
  _udp.endPacket(); 
}


void UDPMessengerService::processMessage(IPAddress senderIp, uint16_t senderPort, char *message)

{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(message);
  if(root.success()) {
    const char* _cmd = root["cmd"]; 
    const char* _serverIP = root["SERVERIP"]; 
    const char* _serverTIME = root["TIME"]; 
    if (strcmp(_cmd,"SHOW")==0){
        serverConnected = true;      
        strcpy(cServerIP,_serverIP);  
        serverIP.fromString(_serverIP);
        unsigned long tmpTime  = strtoul(_serverTIME,NULL,0);
        setTime(tmpTime);
        //now i have to register in Heating server
        registerInServer();
    }
    if (strcmp(_cmd,"RESTART")==0){
        ESP.restart();
      //very simple auth by server IP
    }
    if (strcmp(_cmd,"OK")==0){
      // confirmation of command received;
      if (strtoul(_serverTIME,NULL,0)==_currentCommand){
        //commandConfirmed but there is need to add another field - commnand ID instead of using server time
        _currentCommand = 0;
      }
    }
  

  }

}

void UDPMessengerService::searchForServer(){
  //cmd = SHOWSERVER
  char jasonBuffer[_MAX_PACKET_SIZE] = ""; 
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255;     
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jason = jsonBuffer.createObject();
  jason["cmd"]="SHOWSERVER";
  jason["ID"] = _CLIENT_ID; 
  jason["actualTEMP"] = 0;
  jason["targetTEMP"] = 0;
  jason["serial"] = ESP.getChipId();
  jason["version"] = _CLIENT_VERSION;   
  jason.printTo(jasonBuffer,_MAX_PACKET_SIZE);    
  sendPacket(broadcastIP,true,_HEATING_PORT,jasonBuffer);  
}

time_t UDPMessengerService::sendTempCommand(float actualTemp, float targetTemp, bool switchOn, int thermoID){
  time_t result = 0;
  char jasonBuffer[_MAX_PACKET_SIZE] = "";    
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jason = jsonBuffer.createObject();   
  if (serverConnected) {
    result = millis();
    jason["cmd"]  = switchOn ? "on": "off";
    jason["ID"] = _CLIENT_ID;
    jason["actualTEMP"] = actualTemp;
    jason["targetTEMP"] = targetTemp;
    jason["serial"] = ESP.getChipId();
    jason["version"] = _CLIENT_VERSION;    
    jason.printTo(jasonBuffer,_MAX_PACKET_SIZE);
    _currentCommand = result;
    sendPacket(serverIP,false,_HEATING_PORT,jasonBuffer);
  }
  return result;
}

void UDPMessengerService::registerInServer(){
  char tempBuffer[_MAX_PACKET_SIZE] = "";    
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jason = jsonBuffer.createObject(); 
  jason["cmd"]  = "HELLO";
  jason["ID"] = _CLIENT_ID;
  jason["serial"] = ESP.getChipId();
  jason["version"] = _CLIENT_VERSION;
  jason["actualTEMP"] = 0;
  jason["targetTEMP"] = 0;  
  char hr[21];
  sprintf(hr,"IP :%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);    
  jason["IP"] = hr;  
  jason.printTo(tempBuffer,_MAX_PACKET_SIZE); 
  sendPacket(serverIP,false,_HEATING_PORT,tempBuffer);  
}
