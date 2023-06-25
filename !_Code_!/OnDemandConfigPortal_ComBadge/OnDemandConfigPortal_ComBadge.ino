
// Libraries:
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>            // Capacitive Touch Sensor @ 0x5A
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Arduino.h>

// select which pin will trigger the configuration portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared
// with the bootloader, so always set them HIGH at power-up


// Initillizations:
Adafruit_MPR121 cap = Adafruit_MPR121();      // Capacitive Sensor

uint16_t lasttouched = 0;
uint16_t currtouched = 0;


#define touchPin T2   // GPIO2    TouchValue1
int touchValue1;
//Touch Wake:
#define TOUCH_THRESHOLD 40

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\n Starting");
  Wire.begin();
  pinMode(14, INPUT_PULLDOWN);
  cap.begin(0x5A);                        //       I2C Address: 0x5A
  cap.setThreshholds(12, 6);


  
}


void loop() {
  currtouched = cap.touched();        //Call first thing to read from MPR121
  
  touchValue1 = touchRead(T2);
  // is configuration portal requested?
  //if (digitalRead(touchValue1) == LOW )
  if (touchValue1 < TOUCH_THRESHOLD)
    {
      //WiFiManager
      //Local intialization. Once its business is done, there is no need to keep it around
      WiFiManager wifiManager;

      //reset settings - for testing
      //wifiManager.resetSettings();

      //sets timeout until configuration portal gets turned off
      //useful to make it all retry or go to sleep
      //in seconds
      //wifiManager.setTimeout(120);


      //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
      //WiFi.mode(WIFI_STA);

      if (!wifiManager.startConfigPortal("ComBadge_Setup_AP", "password")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
      }

      //if you get here you have connected to the WiFi
      Serial.println("Connection Established...");
      Serial.print("CONNECTED TO: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP ADDRESS: ");
      Serial.println(WiFi.localIP());
      Serial.print("SIGNAL: ");
      Serial.print(WiFi.RSSI());
    }


    // put your main code here, to run repeatedly:

    lasttouched = currtouched;    //Last call for Capacitive Touch Sensor
  }
