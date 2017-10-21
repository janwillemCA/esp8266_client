/* -*- ESP_IOT -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 2017 Jan Willem Casteleijn
 *   Copyright 20017 Bramboos Media; author J.W.A Casteleijn
 *
 *   version 1.2.1
 *
 *   - using mqqt
 * ----------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MQTTClient.h>

String ipToString(IPAddress ip);
uint32_t get_vcc();
void connect_wifi();
void init_SPI();
int digitalPotWrite(int value);
void messageReceived(String &topic, String &payload);

const char* IP_ADRESS = "192.168.0.110";
const int PORT = 8080;
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
String ipToString(IPAddress ip)
{
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
void connect() 
{
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

  Serial.println("\nconnected!");

  client.subscribe("/hello");
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
  Serial.begin(115200);
  init_SPI();

  WiFi.begin(SSID, PASSWD);
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  //
  // MQTT brokers usually use port 8883 for secure connections.
  client.begin("localbram.bladevm.com", 8443, net);
  client.onMessage(messageReceived);

  connect();
}

void messageReceived(String &topic, String &payload) 
{
  Serial.println("incoming: " + topic + " - " + payload);
}

void loop() 
{
  WiFi.begin(SSID, PASSWD);

  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
  i++;
  delay(250);
  digitalPotWrite(i);
  if(i == 255)
    i = 0;
}
