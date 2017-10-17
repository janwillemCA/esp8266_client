/* -*- ESP_IOT -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 2017 Jan Willem Casteleijn
 *   Copyright 20017 Bramboos Media; author J.W.A Casteleijn
 *
 *   version 1.1
 *
 *   - added 2 seperate functions to send data (info & status)
 * ----------------------------------------------------------------------- */

#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <ArduinoJson.h>


const char* IP_ADRESS = "192.168.0.110";
const int PORT = 8080;
const char * SSID = "DEVKEET";
const char * PASSWD = "wifigratisbord";

extern "C" {
#include "user_interface.h"
}

/* Enable ADC to read voltage level of the ESP8266 */
ADC_MODE(ADC_VCC);

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
StaticJsonBuffer<200> jsonBuffer;

char buffer[256];
const char cmp[] = "whoareyou";
const char status[] = "status";

/* function which converts a typedef IPAdress to a String */
String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

/* function to get voltage level of the ESP8266 */
uint32_t get_vcc()
{
    uint32_t getVcc = ESP.getVcc();
    return getVcc;
}

/* function which sends device information */
void send_info()
{
  JsonObject& json = jsonBuffer.createObject();
  JsonObject& jsonData = json.createNestedObject("data");

  jsonData["device_id"] = ESP.getChipId();
  jsonData["mac_addr"] = WiFi.macAddress();
  jsonData["ip_addr"] = ipToString(WiFi.localIP());

  json["command"] = "iam";

  json.printTo(buffer, sizeof(buffer));
  jsonBuffer.clear();
  webSocket.sendTXT(buffer);
}

/* function whichs sends device status */
void send_status()
{
  JsonObject& json = jsonBuffer.createObject();
  JsonObject& jsonData = json.createNestedObject("data");

  jsonData["vcc_in"] = get_vcc();
  jsonData["cpu_freq"] = ESP.getCpuFreqMHz();

  json["command"] = "status";

  json.printTo(buffer, sizeof(buffer));
  jsonBuffer.clear();
  webSocket.sendTXT(buffer);
}

/* function whichs sends data */
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    break;
  case WStype_CONNECTED:
    break;
  case WStype_TEXT:
		if (strcmp((const char*)payload, cmp) == 0)
      send_info();   /* <-- sends data if server requests data */
    else if(strcmp((const char * )payload, status) == 0)
      send_status(); /* <-- sends data if server requests data */
    delay(1000);
    break;
  case WStype_BIN:
    break;
  }
}

/* connect to network */
void connect_wifi()
{
   //WiFi.persistent(false);
  //  WiFi.forceSleepWake();      // <-- WITHOUT THIS ESP CONNECTS ONLY AFTER FIRST FEW RESTARTS OR DOESN'T CONNECT AT ALL
  WiFiMulti.addAP(SSID, PASSWD);

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(1000);
  }
}

/* initialise websocket */
void init_websocket()
{
  webSocket.begin(IP_ADRESS, PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void setup()
{
  connect_wifi();
  init_websocket();
}

void loop() {
  webSocket.loop();
}
