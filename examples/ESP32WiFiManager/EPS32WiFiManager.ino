#define ESP32
#define DEBUG 

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define LED_NOTIFY_ESP32 21
#define VOLTAGE_SENSE_ESP32 35
#define HTTP_ROUTE "/"
#define HTTP_ROUTE_WIFI "/wifi"
#define HTTP_ROUTE_WIFI_CONFIG "/wifi/config"
#define HTTP_ROUTE_WIFI_DELETE "/wifi/delete"
#define WIFI_NAME "OpenBCI-1234"
#define SOFTWARE_VERSION "v0.0.1"

boolean startWifiManager;
boolean wifiReset;
boolean tryConnectToAP;
boolean ledState;

int ledFlashes;
int ledInterval;
unsigned long ledLastFlash;

WebServer server(80);

String jsonStr;

unsigned long lastSendToClient;
unsigned long lastHeadMove;
unsigned long lastVoltageRead;
unsigned long wifiConnectTimeout;

WiFiUDP clientUDP;
WiFiClient clientTCP;

uint32_t bufferPosition = 0;

///////////////////////////////////////////
// Utility functions
///////////////////////////////////////////

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

///////////////////////////////////////////////////
// MQTT
///////////////////////////////////////////////////


/**
* Used when
*/
void configModeCallback (WiFiManager *myWiFiManager) {
  #ifdef DEBUG
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  #endif
}


///////////////////////////////////////////////////
// HTTP Rest Helpers
///////////////////////////////////////////////////



void sendHeadersForCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void sendHeadersForOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST,DELETE,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "text/plain", "");
}

void requestWifiManagerStart() {
  sendHeadersForCORS();
  // server.send(301, "text/html", "<meta http-equiv=\"refresh\" content=\"1; URL='/'\" />");

  String out = "<!DOCTYPE html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><html lang=\"en\"><h1 style=\"margin:  auto\;width: 80%\;text-align: center\;\">Push The World</h1><br><p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
  if (WiFi.localIP().toString().equals("192.168.4.1") || WiFi.localIP().toString().equals("0.0.0.0")) {
    out += "192.168.4.1";
  } else {
    out += WiFi.localIP().toString();
  }
  out += HTTP_ROUTE;
  out += "'>Click to Go To WiFi Manager</a></p><html>";
  server.send(200, "text/html", out);

  ledFlashes = 5;
  ledInterval = 250;
  ledLastFlash = millis();

  startWifiManager = true;
}

JsonObject& getArgFromArgs(int args) {
  DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(args) + (40 * args));
  JsonObject& root = jsonBuffer.parseObject(server.arg(0));
  return root;
}

JsonObject& getArgFromArgs() {
  return getArgFromArgs(1);
}

void removeWifiAPInfo() {
  // wifi.curClientResponse = wifi.CLIENT_RESPONSE_OUTPUT_STRING;
  // wifi.outputString = "Forgetting wifi credentials and rebooting";
  // wifi.clientWaitingForResponseFullfilled = true;

// #ifdef DEBUG
//   Serial.println(wifi.outputString);
//   Serial.println(ESP.eraseConfig());
// #else
//   ESP.eraseConfig();
// #endif
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  delay(1000);
}



void initializeVariables() {
  ledState = false;
  startWifiManager = false;
  tryConnectToAP = false;
  wifiReset = false;

  lastHeadMove = 0;
  lastSendToClient = 0;
  lastVoltageRead = 0;
  ledFlashes = 0;
  ledInterval = 300;
  ledLastFlash = millis();
  wifiConnectTimeout = millis();

  jsonStr = "";
}

void setup() {
  initializeVariables();

  WiFi.mode(WIFI_AP_STA);
  // WiFi.mode(WIFI_STA);
  // WiFi.mode(WIFI_AP);

  #ifdef DEBUG
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Serial started");
  #endif

  pinMode(LED_NOTIFY_ESP32, OUTPUT);


#ifdef DEBUG
  Serial.println("SPI Slave ready");
  printWifiStatus();
  Serial.printf("Starting HTTP...\n");
#endif

  server.on(HTTP_ROUTE, HTTP_GET, [](){
    String ip = "192.168.4.1";
    String out = "<!DOCTYPE html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><html lang=\"en\"><h1 style=\"margin:  auto\;width: 90%\;text-align: center\;\">Neurosity, Inc.</h1><br>";
    if (WiFi.localIP().toString().equals("192.168.4.1") || WiFi.localIP().toString().equals("0.0.0.0")) {
      if (WiFi.SSID().equals("")) {
        out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
        out += "192.168.4.1";
        out += HTTP_ROUTE_WIFI_CONFIG;
        out += "'>Click to Configure Wifi</a><br>If the above link does not work type 192.168.4.1/wifi in web browser and press Enter or Go.<br>See updates on issue <a href='https://github.com/OpenBCI/OpenBCI_WIFI/issues/62'>#62</a> on Github.</p><br>";
      } else {
        out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
        out += "192.168.4.1";
        out += HTTP_ROUTE_WIFI_CONFIG;
        out += "'>Click to Configure Wifi</a><br>If the above link does not work type 192.168.4.1/wifi in web browser and press Enter or Go.<br>See updates on issue <a href='https://github.com/OpenBCI/OpenBCI_WIFI/issues/62'>#62</a> on Github.</p><br>";
        out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
        out += "192.168.4.1";
        out += HTTP_ROUTE_WIFI_DELETE;
        out += "'>Click to Erase Wifi Credentials</a></p><br>";
      }
    } else {
      out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
      out += WiFi.localIP().toString();
      out += HTTP_ROUTE_WIFI_CONFIG;
      out += "'>Click to Configure Wifi</a><br>If the above link does not work type ";
      out += WiFi.localIP().toString();
      out += "/wifi in web browser and press Enter or Go.<br>See updates on issue <a href='https://github.com/OpenBCI/OpenBCI_WIFI/issues/62'>#62</a> on Github.</p><br>";
      out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
      out += WiFi.localIP().toString();
      out += HTTP_ROUTE_WIFI_DELETE;
      out += "'>Click to Erase Wifi Credentials</a></p><br>";
    }
    out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"><a href='http://";
    if (WiFi.localIP().toString().equals("192.168.4.1") || WiFi.localIP().toString().equals("0.0.0.0")) {
      out += "192.168.4.1/update";
    } else {
      out += WiFi.localIP().toString();
      out += "/update";
    }
    out += "'>Click to Update WiFi Firmware</a></p><br>";
    out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"> Please visit <a href='https://app.swaggerhub.com/apis/pushtheworld/openbci-wifi-server/2.0.0'>Swaggerhub</a> for the latest HTTP endpoints</p><br>";
    out += "<p style=\"margin:  auto\;width: 80%\;text-align: center\;\"> Shield Firmware: " + String(SOFTWARE_VERSION) + "</p></html>";

    server.send(200, "text/html", out);
  });
  server.on(HTTP_ROUTE, HTTP_OPTIONS, sendHeadersForOptions);

  if (!MDNS.begin(WIFI_NAME)) {
#ifdef DEBUG
    Serial.println("Error setting up MDNS responder!");
#endif
  } else {
#ifdef DEBUG
    Serial.print("Your ESP is called "); Serial.println(WIFI_NAME);
#endif
  }

  server.onNotFound([](){
#ifdef DEBUG
    Serial.println("HTTP NOT FOUND " + server.uri());
#endif
    server.send(404, "text/html", "Route Not Found");
  });

  server.on(HTTP_ROUTE_WIFI, HTTP_GET, requestWifiManagerStart);
  server.on(HTTP_ROUTE_WIFI, HTTP_DELETE, []() {
    server.send(200, "text/html", "Reseting wifi. Please power cycle your board in 10 seconds");
    wifiReset = true;
  });
  server.on(HTTP_ROUTE_WIFI, HTTP_OPTIONS, sendHeadersForOptions);

  server.on(HTTP_ROUTE_WIFI_DELETE, HTTP_GET, []() {
    server.send(200, "text/html", "Reseting wifi. Please power cycle your board in 10 seconds");
    wifiReset = true;
    digitalWrite(LED_NOTIFY_ESP32, LOW);
  });
  server.on(HTTP_ROUTE_WIFI_DELETE, HTTP_OPTIONS, sendHeadersForOptions);

#ifdef DEBUG
  Serial.printf("Ready!\n");
#endif

  if (WiFi.SSID().equals("")) {
    WiFi.softAP(WIFI_NAME);
    WiFi.mode(WIFI_AP);
#ifdef DEBUG
    Serial.printf("No stored creds, turning wifi into access point with %d bytes on heap\n", ESP.getFreeHeap());
#endif
    // httpUpdater.setup(&server);
    server.begin();
    MDNS.addService("http", "tcp", 80);
    ledFlashes = 10;
    ledInterval = 100;
    ledLastFlash = millis();
    ledState = false;
    // digitalWrite(LED_NOTIFY_ESP32, HIGH);
  } else {
    ledState = false;
    ledFlashes = 4;
    ledInterval = 250;
    wifiConnectTimeout = millis();
    tryConnectToAP = true;
#ifdef DEBUG
    Serial.printf("Stored creds, will try to connect for 10 seconds with %d bytes on heap\n", ESP.getFreeHeap());
#endif
  }

#ifdef DEBUG
  Serial.printf("END OF SETUP HEAP: %d\n", ESP.getFreeHeap());
#endif
}

/////////////////////////////////
/////////////////////////////////
// LOOP LOOP LOOP LOOP LOOP /////
/////////////////////////////////
/////////////////////////////////
void loop() {

  if (ledFlashes > 0) {
    if (millis() > (ledLastFlash + ledInterval)) {
      digitalWrite(LED_NOTIFY_ESP32, ledState ? HIGH : LOW);
      if (ledState) {
        ledFlashes--;
      }
      ledState = !ledState;
      ledLastFlash = millis();
    }
  }

  if (!tryConnectToAP) {
    server.handleClient();
  }

  if (wifiReset) {
#ifdef DEBUG
    Serial.println("WiFi Reset");
#endif
    WiFi.mode(WIFI_STA);
    wifiReset = false;
    delay(1000);
    WiFi.disconnect();
    delay(1000);
    ESP.restart();
    // hard_restart();
  }

  if (tryConnectToAP) {
    if (WiFi.status() == WL_CONNECTED) {
      tryConnectToAP = false;
      WiFi.mode(WIFI_STA);
      // httpUpdater.setup(&server);
      server.begin();
      MDNS.addService("http", "tcp", 80);
      // digitalWrite(LED_NOTIFY_ESP32, HIGH);
#ifdef DEBUG
      Serial.println("Connected to network, switching to station mode.");
#endif
      ledState = false;
      ledFlashes = 2;
      ledInterval = 500;
      ledLastFlash = millis();
  } else if (millis() > (wifiConnectTimeout + 10000)) {
#ifdef DEBUG
      Serial.printf("Failed to connect to network with %d bytes on head\n", ESP.getFreeHeap());
#endif
      tryConnectToAP = false;
      WiFi.mode(WIFI_AP);
#ifdef DEBUG
      Serial.printf("Started AP with %d bytes on head\n", ESP.getFreeHeap());
#endif
      // httpUpdater.setup(&server);
      server.begin();
      MDNS.addService("http", "tcp", 80);
      ledFlashes = 10;
      ledInterval = 100;
      ledLastFlash = millis();
    }
  }

 

  if (startWifiManager) {
    startWifiManager = false;

// #ifdef DEBUG
//     Serial.printf("%d bytes at start of wifi manager\n", ESP.getFreeHeap());
// #endif
#ifdef DEBUG
    Serial.printf("%d bytes on heap before stopping local server\n", ESP.getFreeHeap());
#endif
    server.stop();
    
    delay(100);
#ifdef DEBUG
    Serial.printf("%d bytes on after stopping local server\n", ESP.getFreeHeap());
#endif
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    WiFiManagerParameter custom_text("<p>Powered by Neurosity, Inc.</p>");
    wifiManager.addParameter(&custom_text);
#ifdef DEBUG
    Serial.printf("Start WiFi Config Portal on WiFi Manager with %d bytes on heap\n" , ESP.getFreeHeap());
#endif
    boolean connected = wifiManager.startConfigPortal(WIFI_NAME);
#ifdef DEBUG
    if (connected) {
      Serial.printf("Connected to with WiFi Manager with %d bytes on heap\n" , ESP.getFreeHeap());
    } else {
      Serial.printf("Failed to connect with WiFi Manager with %d bytes on heap\n" , ESP.getFreeHeap());
    }
#endif
  }
}
