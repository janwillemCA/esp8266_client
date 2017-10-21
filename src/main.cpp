/* -*- ESP_IOT -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 2017 Jan Willem Casteleijn
 *   Copyright 2017 Bramboos Media; author J.W.A Casteleijn
 *
 *   version 1.2.1
 *
 *   - MQQT implemented
 * ----------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MQTTClient.h>

#define MQQT_CLIENT_ID  (String(ESP.getChipId(), HEX)).c_str() + (String("/connected"))
#define MQQT_DIM  (String(ESP.getChipId(), HEX)).c_str() + (String("/dim"))

String ipToString(IPAddress ip);
uint32_t get_vcc();
void connect_wifi();
void init_SPI();
int digitalPotWrite(int value);
void messageReceived(String &topic, String &payload);

const char * SSID = "DEVKEET";
const char * PASSWD = "wifigratisbord";

unsigned long lastMillis = 0;

/* SPI potentiometer */
byte address = 0x11;
int CS= 15;
int i = 0;

/* Enable ADC to read voltage level of the ESP8266 */
ADC_MODE(ADC_VCC);

WiFiClientSecure net;
MQTTClient client;

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

/* connect to network */
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
  // client.unsubscribe("/hello");
}


void init_SPI()
{
  Serial.println("Init SPI.....");
  pinMode (CS, OUTPUT);
  SPI.begin();

  digitalPotWrite(0x00);
  delay(1000);

  digitalPotWrite(0x80);
  delay(1000);

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

void setup() {
  Serial.begin(115200);
  init_SPI();
  WiFi.begin(SSID, PASSWD);

  client.begin("localbram.bladevm.com", 8443, net);
  client.onMessage(messageReceived);

  connect();

  client.publish(MQQT_CLIENT_ID);
  client.subscribe(MQQT_DIM);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void loop() {
  WiFi.begin(SSID, PASSWD);
  client.loop();

  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  /* mcp11xxxx demo */
  i++;
  delay(250);
  digitalPotWrite(i);
  if(i == 255)
    i = 0;
}
