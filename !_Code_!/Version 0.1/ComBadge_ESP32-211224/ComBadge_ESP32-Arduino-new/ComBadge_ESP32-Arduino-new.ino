/* 
    ComBadge V0.1a Arduino C++
*/


// Libraries:

// File System:// File System & File Browser:
#include "FS.h"
#include <LittleFS.h>
#include <EEPROM.h>
#include <ESPxWebFlMgr.h>
#include "ArduinoJson.h"  //https://github.com/bblanchon/ArduinoJson

// Main System:
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>            // Capacitive Touch Sensor @ 0x5A
//#include "Arduino-MAX17055_Driver.h"    // Battery monitor
#include "Arduino.h"
#include "esp_sleep.h"

// Audio:
#include "AudioConfig.h"
#include "AudioLogger.h"
#include "AudioTools.h"   //https://github.com/pschatzmann/arduino-audio-tools
#include <DacESP32.h>

// Wifi:
#include <WiFi.h>
#include "WebServer.h"    //https://github.com/zhouhan0126/WebServer-esp32
#include "DNSServer.h"    //https://github.com/zhouhan0126/DNSServer---esp32
#include <WiFiManager.h>  //https://github.com/ozbotics/WIFIMANAGER-ESP32
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"


// Deffinitions:

//TouchPins:                                        dev cable colour:             Function:
#define touchPin2 T2   // GPIO2   TouchShift         WHITE                        
#define touchPin1 T5   // GPIO12   TouchMain          YELLOW  
#define touchPin3 T7   // GPIO27   TouchMiddle        BLUE                        
#define touchPin4 T8   // GPIO33   TouchBottom        GREEN                                             
touch_pad_t wakePin;

#define null 0

//Touch Settings (Internal):
const int threshold = 40;
#define SENS_TOUCH_MEAS_EN = 5
int TouchMain;
int TouchShift;
int TouchMiddle;
int TouchBottom;

//I2S:
#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14

// Use ESP32 I2S Processor 0
#define I2S_PORT I2S_NUM_0
#define bufferLen 64
int16_t sBuffer[bufferLen];

//DAC/Tones:
//#define DAC1 25
DacESP32 dac1(GPIO_NUM_25);
#define ns8002_mute 13
#define Tones 26

// SPI microSD card:
//#define sd_sclk 18
//#define sd_mosi 23
//#define sd_miso 19
//#define sd_dc   16
//#define sd_rst  5
//#define sd_cs   4


// ~~~~~~~~~~~~~~~~~~~~~~~~~~FIRMWARE VERSION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Current Version to compare to Server Version (!) 0.1 for base testing(!)
String FirmwareVer = "0.2";
//{
//    "0.1"
//};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Text file containing Version Number:
//#define URL_fw_Version "https://github.com/DansDesigns/ComBadge/blob/main/firmware/version.txt"
#define URL_fw_Version "https://raw.githubusercontent.com/DansDesigns/ComBadge/main/firmware/version.txt"

// .bin file if New Version
//#define URL_fw_Bin "https://github.com/DansDesigns/ComBadge/raw/main/firmware/fw.bin"
#define URL_fw_Bin "https://raw.githubusercontent.com/DansDesigns/ComBadge/main/firmware/fw.bin"

//Other locations for firmware hosting..
//#define URL_fw_Version "http://cade-make.000webhostapp.com/version.txt"
//#define URL_fw_Bin "http://cade-make.000webhostapp.com/firmware.bin"

// Initillizations:

//Touch Settings (MPR121):
Adafruit_MPR121 cap = Adafruit_MPR121();      // Capacitive Sensor
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// Variables:

//State Counters:
int touch_counter = 0;
int cycle_counter = 0;
int wake_state = 0;

const char* ssid = "Default SSID";
const char* passphrase = "Default passord";

const word filemanagerport = 8080;

ESPxWebFlMgr filemgr(filemanagerport); // we want a different port than the webserver

int i = 0;
int statusCode;
String st;
String content;
String esid;
String epass = "";

//Function Decalrations
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80
WebServer server(80);

//flag for saving data
bool shouldSaveConfig = false;


//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void callback() {}





//EOF End-Of-File
