#define ESP32
#define BUFFER_SIZE 1440

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SlaveSPIClass.h>
// #include <HTTPClient.h>
// #include <ESP32httpUpdate.h>
#include "OpenBCI_Wifi_Definitions.h"
#include "OpenBCI_Wifi.h"
#define SO 17
#define SI 4
#define SCLK 16
#define SS 34
#define LED_NOTIFY_ESP32 21
#define VOLTAGE_SENSE_ESP32 35

SlaveSPI slave;
void test();
String perfectPrintByteHex(uint8_t b) {
  if (b <= 0x0F) {
    return "0" + String(b, HEX);
  } else {
    return String(b, HEX);
  }
}
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  slave.begin((gpio_num_t)SO,(gpio_num_t)SI,(gpio_num_t)SCLK,(gpio_num_t)SS,32,test);//seems to work with groups of 4 bytes
}
String txt = "";
String cmd ="";
void loop() {
  // put your main code here, to run repeatedly:
  if(slave.getBuff()->length()&&digitalRead(SS)==HIGH) {
    while(slave.getBuff()->length())
      txt+=slave.read();
    Serial.println("slave input:");
    for(int i=0;i<txt.length();i++)
      Serial.print(perfectPrintByteHex(txt[i]));
    Serial.println();
  }
  while(Serial.available()) {
    cmd +=(char) Serial.read();
  }
  while(txt.length()>0) {
    slave.trans_queue(txt);
    Serial.println("slave output:");
    for(int i=0;i<txt.length();i++)
      Serial.print(perfectPrintByteHex(txt[i]));
    Serial.println();
    // Serial.println(txt);
    txt ="";
  }
  while(cmd.length()>0) {
    Serial.println("input:");
    Serial.println(cmd);

    for(int i=0;i<cmd.length();i++)
      Serial.print(perfectPrintByteHex(cmd[i]));
    cmd ="";
  }
}

void test() {
  //Serial.println("test");
  //Serial.println(slave[0]);
}



