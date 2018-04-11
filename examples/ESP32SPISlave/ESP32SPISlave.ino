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
#define SPI_BUFFER_LENGTH 34


SlaveSPI slave;
String txt = "";
String cmd ="";
boolean statusNeedsToBeSent = false;
boolean streamStart = false;
boolean wasData = false;
boolean wasPolled = false;
unsigned long lastPacketArrival = 0;
uint8_t buffer[SPI_BUFFER_LENGTH];
volatile boolean newData = false;

void test() {
  // Serial.println("SPI Slave Data sent");
  // txt = "";
  // Serial.println(slave[0]);
  newData = true;
}

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
  slave.begin((gpio_num_t)SO, (gpio_num_t)SI, (gpio_num_t)SCLK, (gpio_num_t)SS, 32, test);//seems to work with groups of 4 bytes
  wifi.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  if (newData) {
    newData = false;
    Serial.print("New Data:\n  ");
    for(int i=0;i<32;i++){
      Serial.print(perfectPrintByteHex(slave.bufferRx[i]));
    }
    Serial.println();
    // while(slave.getBuff()->length())
    //   txt+=slave.read();
    // Serial.println("slave input:");
    if (slave.bufferRx[0] == 0x04) {
      Serial.println("Status!");
      
      statusNeedsToBeSent = true;
      for(int i = 0; i < SPI_BUFFER_LENGTH; i++) {
        buffer[i] = 0;
      }
      buffer[1] = 0xD1;
      slave.trans_queue(buffer, 32);
      // slave.setStatus(209);
    } else if (slave.bufferRx[0] == 0x03) {
      wasPolled = true;

    } else if (slave.bufferRx[0] == 0x02) {
      wasData = true;
      for (int i = 0; i < 32; i++) {
        buffer[i] = slave.bufferRx[i+2];
      }
    // } else {
    //   Serial.println("Something else");
    //   for(int i=0;i<txt.length();i++)
    //     Serial.print(perfectPrintByteHex(txt[i]));
    //   Serial.println();
    }

  }
  while(Serial.available()) {
    cmd +=(char) Serial.read();
  }
  if(wasData || wasPolled) {
    // if (statusNeedsToBeSent) {
    //   Serial.println("Fixing status");
    //   statusNeedsToBeSent = false;
    //   txt[0]=209;
    //   txt[1]=209;
    //   txt[2]=0;
    //   txt[3]=0;
    //   txt[4]=0;
    //   slave.trans_queue(txt);
    // }
    if (wasPolled) {
      wasPolled = false;
      if (!streamStart) {
        Serial.println("Starting stream");
        streamStart = true;
        // txt[0]=0;
        // txt[1]=0;
        // txt[2]=1;
        // txt[3]='b';
        // slave.trans_queue(txt);

        wifi.passthroughCommands("b");
        slave.trans_queue(wifi.passthroughBuffer, 32);
        // txt[0] = 1;
        // txt[1] = 'b';        
      }
      // wifi.passthroughBufferClear();
      // slave.trans_queue(wifi.passthroughBuffer, 32);
    }
    if (wasData) {
      wasData = false;
      unsigned long curTime = millis();
      Serial.printf("%d\n", curTime - lastPacketArrival);
      for(int i=0;i<32;i++)
        Serial.print(perfectPrintByteHex(slave.bufferRx[i+2]));
      lastPacketArrival = millis();
      Serial.println();
      
    }
    if (wifi.passthroughPosition > 0) {
      Serial.println("Pass through slave output:");
      for(int i=0;i<32;i++)
        Serial.print(perfectPrintByteHex(wifi.passthroughBuffer[i]));
      Serial.println();
    } else if (txt.length() > 0) {
      // Serial.println("Txt slave output:");
      // for(int i=0;i<txt.length();i++)
      //   Serial.print(perfectPrintByteHex(txt[i]));
      // Serial.println();
    } 
    // Serial.println(txt);
    txt = "";
    wifi.passthroughBufferClear();
  }
  while(cmd.length()>0) {
    Serial.println("input:");
    Serial.println(cmd);

    for(int i=0;i<cmd.length();i++)
      Serial.print(perfectPrintByteHex(cmd[i]));
    cmd ="";
  }
}
