/**
 * @file streams-i2s-i2s.ino
 * @brief Copy audio from I2S to mqtt server
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
 
// Libraries:
#include "FS.h"           // ESP-32 File System
#include "SPIFFS.h"       // ESP-32 File System drivers
#include "ArduinoJson.h"  //https://github.com/bblanchon/ArduinoJson

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>            // Capacitive Touch Sensor @ 0x5A
#include "Arduino.h"
#include <SoftwareSerial.h>

#include "AudioConfig.h"
#include "AudioLogger.h"
#include "AudioTools.h"   //https://github.com/pschatzmann/arduino-audio-tools

#include <WiFi.h>
#include "WebServer.h"    //https://github.com/zhouhan0126/WebServer-esp32
#include "DNSServer.h"    //https://github.com/zhouhan0126/DNSServer---esp32
#include <WiFiManager.h>  //https://github.com/ozbotics/WIFIMANAGER-ESP32

#include "MQTT.h"         // https://github.com/256dpi/arduino-mqtt


const char ssid[] = "Workshop-Wifi";
const char pass[] = "Tintin10";

MQTTClient mqtt;


uint16_t sample_rate=44100;
uint16_t channels = 1;
uint16_t bits_per_sample = 16; // or try with 24 or 32
I2SStream i2s;
WiFiClient client;                  
MeasuringStream clientTimed(client);
StreamCopy copier(clientTimed, i2s); // copy (TO HERE, from HERE) (####, ####)

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!mqtt.connect("Badge_Test", "dan", "Tintin10")) {    // ClientID, Username, Password for MQTT Server
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  mqtt.subscribe("/badges");
  mqtt.publish("/badges", "Hey");
  //mqtt.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}


// Arduino Setup
void setup(void) {  
  
  WiFi.begin(ssid, pass);
  Serial.begin(115200);
  // change to Warning to improve the quality
  AudioLogger::instance().begin(Serial, AudioLogger::Info); 

  // start I2S in
  Serial.println("starting I2S...");
  auto config = i2s.defaultConfig(RXTX_MODE);
  config.sample_rate = sample_rate; 
  config.bits_per_sample = bits_per_sample; 
  config.channels = 2;
  config.i2s_format = I2S_STD_FORMAT;
  config.pin_ws = 15;
  config.pin_bck = 14;
  config.pin_data = 32;
  //config.pin_data_rx = 19;
  //config.fixed_mclk = sample_rate * 256;
  // config.pin_mck = 3; // must be 0,1 or 3 - only for ESP_IDF_VERSION_MAJOR >= 4
  i2s.begin(config);
  Serial.println("I2S started...");
  
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  
  mqtt.begin("192.168.1.165", client);      // Broker Domain/IP Address, Network Interface to connect with
  mqtt.onMessage(messageReceived);

  connect();
  
}

// Arduino loop - copy sound to out 
void loop() {
  copier.copy();
}
