/* 
    ComBadge mk1 Arduino C++
      Last Edit: 30/07/2023  
      
*/


// Libraries:
#include "FS.h"           // ESP-32 File System
#include "SPIFFS.h"       // ESP-32 File System drivers
#include "ArduinoJson.h"  //https://github.com/bblanchon/ArduinoJson

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>            // Capacitive Touch Sensor @ 0x5A
#include "Arduino.h"
//#include <SoftwareSerial.h>     // DOESNT COMPILE ON WINDOWS?

#include "AudioConfig.h"
#include "AudioLogger.h"
#include "AudioTools.h"   //https://github.com/pschatzmann/arduino-audio-tools

#include <WiFi.h>
#include "WebServer.h"    //https://github.com/zhouhan0126/WebServer-esp32
#include "DNSServer.h"    //https://github.com/zhouhan0126/DNSServer---esp32
#include <WiFiManager.h>  //https://github.com/ozbotics/WIFIMANAGER-ESP32

// Deffinitions:

//TouchPins:                                        dev cable colour:
#define touchPin T5   // GPIO12   Wake On Touch       Yellow (no plastic sleve)
#define touchPin T2   // GPIO2    TouchValue1         Yellow
#define touchPin T7   // GPIO27   TouchValue2         WHITE
#define touchPin T8   // GPIO33   TouchValue3         ORANGE

touch_pad_t wakePin;

//I2S:
#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14

// Use ESP32 I2S Processor 0
#define I2S_PORT I2S_NUM_0
#define bufferLen 64
int16_t sBuffer[bufferLen];

//DAC/Tones:
#define DAC1 25
#define ns8002_mute 13
#define Tones 26

// Initillizations:

//Touch Settings (Internal):
const int threshold = 40;
int touchValue0;
int touchValue1;
int touchValue2;
int touchValue3;

//Touch Settings (MPR121):
Adafruit_MPR121 cap = Adafruit_MPR121();      // Capacitive Sensor
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

//State Counters:
int touch_counter = 0;
int cycle_counter = 0;
int wake_state = 0;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void callback() {}





//EOF End-Of-File
