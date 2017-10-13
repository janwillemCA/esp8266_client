/* -*- ESP_IOT -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 2017 Jan Willem Casteleijn
 *   Copyright 2017 Bramboos Media; author J.W.A Casteleijn
 *
 *   version 1.0
 * ----------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <ArduinoJson.h>

const char * IP_ADRESS = "192.168.0.110";
const int PORT = 8080;
const char * SSID = "DEVKEET";
const char * PASSWD = "wifigratisbord";

/*
 * Enable ADC converter
 * and SET VCC_IN Read = TRUE to measure Voltage level of the ESP8266
 */
ADC_MODE(ADC_VCC);

/* Create instances of libraries */
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

/* Initialize buffer for json data */
StaticJsonBuffer < 200 > jsonBuffer;

char buffer[256];
char cmp[] = "whoareyou";
float * vcc = 0;

/* This function converts a typdef IPAddress to a string*/
/* Also returns the string as s */
String ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++)
    s += i ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

/* get voltage level of the ESP8266 */
uint32_t get_vcc() {
  uint32_t getVcc = ESP.getVcc();
  return getVcc;
}

/* Function whichs initializes the json data to send */
void send_data() {
  JsonObject & json = jsonBuffer.createObject();
  JsonObject & jsonData = json.createNestedObject("data");

  jsonData["device_id"] = ESP.getChipId();
  jsonData["mac_addr"] = WiFi.macAddress();
  jsonData["ip_addr"] = ipToString(WiFi.localIP());
  jsonData["vcc_in"] = get_vcc();

  json["command"] = "iam";
  json.printTo(buffer, sizeof(buffer));
  jsonBuffer.clear();
  webSocket.sendTXT(buffer);
}

/* send data */
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    break;
  case WStype_CONNECTED:
    break;
  case WStype_TEXT:
    if (strcmp((const char * ) payload, cmp) == 0)
      send_data();    /* <-- sends data if server requests data */
    delay(1000);
    break;
  case WStype_BIN:
    break;
  }
}

/* Simple connect function */
void connect_wifi() {
  WiFiMulti.addAP(SSID, PASSWD);

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(1000);
  }
}

/* Initialize websocket */
void init_websocket() {
  webSocket.begin(IP_ADRESS, PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void setup() {
  init_websocket();
}

void loop() {
  webSocket.loop();
}
