/* -*- ESP_IOT -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 2017 Jan Willem Casteleijn
 *   Copyright 20017 Bramboos Media; author J.W.A Casteleijn
 *
 *   version 1.2
 *
 *   - added 2 seperate functions to send data (info & status)
 * ----------------------------------------------------------------------- */

#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include <SPI.h>

String ipToString(IPAddress ip);
uint32_t get_vcc();
void send_info();
void send_status();
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
void connect_wifi();
void init_websocket();
void init_SPI();
int digitalPotWrite(int value);

const char* IP_ADRESS = "192.168.0.110";
const int PORT = 8080;
const char * SSID = "DEVKEET";
const char * PASSWD = "wifigratisbord";

/* SPI potentiometer */
byte address = 0x11;
int CS= 15;
int i=0;

/* sleep test */
extern "C" {
#include "user_interface.h"
}

/* Enable ADC to read voltage level of the ESP8266 */
ADC_MODE(ADC_VCC);

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
StaticJsonBuffer<200> jsonBuffer;
StaticJsonBuffer<200> parseBuffer;


JsonObject& json = jsonBuffer.createObject();
JsonObject& jsonData = json.createNestedObject("data");

char buffer[256];
char infoBuff[256];
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

  jsonData["device_id"] = ESP.getChipId();
  jsonData["mac_addr"] = WiFi.macAddress();
  jsonData["ip_addr"] = ipToString(WiFi.localIP());

  json["command"] = "auth";

  json.printTo(infoBuff, sizeof(infoBuff));
  //jsonBuffer.clear();
  webSocket.sendTXT(infoBuff);
}

/* function whichs sends device status */
void send_status()
{
  //JsonObject& json = jsonBuffer.createObject();
  //JsonObject& jsonData = json.createNestedObject("data");
  jsonData["vcc_in"] = get_vcc();
  jsonData["cpu_freq"] = ESP.getCpuFreqMHz();

  json["command"] = "status";

  json.printTo(buffer, sizeof(buffer));
  jsonBuffer.clear();
  webSocket.sendTXT(buffer);
}

void parsejson(uint8_t *payload1)
{
  JsonObject& jsonpayload = parseBuffer.parseObject(payload1);
  parseBuffer.clear();

  if(!jsonpayload.success()) {
      webSocket.sendTXT("FOUT!!!!!!");
  }
  else if(jsonpayload["command"] == "auth")
    send_info();
  else if(jsonpayload["command"] == "status") {
    send_status();
  }
  else if(jsonpayload["command"] == "dim") {
    jsonpayload["data"]["value"];
    int pwm = jsonpayload["data"]["value"];

    Serial.print("Value: ");
    Serial.println(pwm);

    digitalPotWrite(pwm);
  }
}

/* function whichs sends data */
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    break;
  case WStype_CONNECTED:
    break;
  case WStype_TEXT:
    Serial.printf("get Text: %s\n", payload);
    parsejson(payload);
    break;
  case WStype_BIN:
    break;
  }
}

/* connect to network */
void connect_wifi()
{
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

void init_SPI()
{
  pinMode (CS, OUTPUT);
  SPI.begin();

  digitalPotWrite(0x00);
  delay(1000);

  digitalPotWrite(0x80);
  delay(1000);

   // adjust Lowest Resistance .
  digitalPotWrite(0xFF);
  delay(1000);
}

int digitalPotWrite(int value)
{
  digitalWrite(CS, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}

void setup()
{
  connect_wifi();
  init_websocket();
  init_SPI();
  Serial.begin(115200);
}

void loop() {
  webSocket.loop();
}
