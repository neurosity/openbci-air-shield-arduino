#########################################
#                                       #
#      Make OpenBCI Air Module         #
#                                       #
#########################################

#########################################
#            makeEspArduino Source      #
#########################################
# from: https://github.com/plerup/makeEspArduino
makeEspArduino = $(HOME)/makeEspArduino/makeEspArduino.mk

#########################################
#            Arduino Lib                #
#########################################
ARDUINO_LIBS = $(HOME)/Documents/Arduino/libraries

#########################################
#            OpenBCI Lib                #
#########################################
OPENBCI_WIFI_DIR = $(ARDUINO_LIBS)/OpenBCI_Wifi
OPENBCI_WIFI_LIB = $(OPENBCI_WIFI_DIR)/src
OPENBCI_AIR_DIR = $(ARDUINO_LIBS)/OpenBCI_Air_Shield_Library
OPENBCI_AIR_LIB = $(OPENBCI_AIR_DIR)/src

#########################################
#            Arduino Sketch             #
#########################################
SKETCH = $(OPENBCI_AIR_DIR)/examples/ESP32WifiShield/ESP32WifiShield.ino


#########################################
#            ESP32 Lib                #
#########################################
ESP_ROOT = $(HOME)/esp32
ESP_LIBS = $(ESP_ROOT)/libraries

#########################################
#            Board Info                 #
#########################################
HTTP_ADDR=OpenBCI-2114.local
UPLOAD_PORT = /dev/ttyUSB0
CHIP = esp32
# BOARD = huzzah
# FLASH_DEF = 4M1M
# F_CPU = 80000000l


#########################################
#            Include/Exclude Libs       #
#########################################
LIBS = $(OPENBCI_AIR_LIB) \
       $(OPENBCI_WIFI_LIB) \
       $(ARDUINO_LIBS)/WiFiManager \
       $(ARDUINO_LIBS)/ArduinoJson \
       $(ARDUINO_LIBS)/Time \
       $(ARDUINO_LIBS)/NTPClient \
       $(ESP_LIBS)/Wire \
       $(ESP_LIBS)/WiFi  \
       $(ESP_LIBS)/FS  \
       $(ESP_LIBS)/ESPWebServer  \
       $(ESP_LIBS)/DNSServer  \
       $(ESP_LIBS)/ESPmDNS \
       $(ESP_LIBS)/Update \
       $(ESP_LIBS)/ArduinoOTA

EXCLUDE_DIRS = $(ARDUINO_LIBS)/ArduinoJson/test \
	       $(ARDUINO_LIBS)/ArduinoJson/third-party \
	       $(ARDUINO_LIBS)/ArduinoJson/fuzzing \
	       $(ARDUINO_LIBS)/WebSockets/examples \
	       $(ARDUINO_LIBS)/PubSubClient/tests \
	       $(ARDUINO_LIBS)/OpenBCI_WIFI/test \
	       $(ARDUINO_LIBS)/OpenBCI_WIFI/tests-ptw-assert \
	       $(ARDUINO_LIBS)/OpenBCI_WIFI/extras \
           $(ARDUINO_LIBS)/OpenBCI_Air_Library/test \
	       $(ARDUINO_LIBS)/OpenBCI_Air_Library/tests-ptw-assert \
	       $(ARDUINO_LIBS)/OpenBCI_Air_Library/extras

include $(makeEspArduino)
