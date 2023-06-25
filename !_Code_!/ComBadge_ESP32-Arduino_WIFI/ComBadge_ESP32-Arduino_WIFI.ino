

/* ComBadge mk1 Arduino C++

  last edit 27/05/2023


  Sketch uses 307873 bytes (9%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 17628 bytes (5%) of dynamic memory, leaving 310052 bytes for local variables. Maximum is 327680 bytes.


   FDTI: COM14
   DevBoard: COM15

   Settings for Arduino IDE:
   BOARD:           ESP32-WROOM-(32)DA
   UPLOAD SPEED:    921600
   CPU FREQ:        240mhz
   FLASH SIZE:      4mb(Default)
   FLASH MODE:      QIO
   PARTITION:       Default



  TO DO:

   ESP32-WROOM-32E - 2Mb Flash    -   Code Checklist:
   BadgeID                            -  DONE
   OEP3W/NS8002 DAC amp               -  DONE
   Power Supply                       -  DRAW ~ 6-15mAh/hour in sleep... 150mAh Peak @ Boot... Wifi > 130mah... Wake from Timer = 96mAh... MQTT Transmission & Recception UNTESTED...
   INMP441 I2S Mic                    -  WORKING
   Mic Stream to IP Address           -
   Stream IP Address to Speaker       -  
   OTA Update                         -
   Chirp on Tap                       -  DONE
   Chirp on Wake                      -  DONE
   Shift Button Wake & Combination    -  DONE
   WIFI Manager                       -  DONE
   On-Demand WIFI AP                  -  DONE
   Save WIFI Credentials              -  
   WIFI AutoConnect from saved info   -    
   Wifi Re-Connect on Wake            -  
   MQTT Server Connecction            -   
   "Save Contact" duplex with Server  -  
   "Record" Function                  -  
   



   Pin Definitions = GPIO-Pin, (Board-Pin #), (SMD Pins in EasyEDA are different, refer to paper-chart pencil numbers):

   Touch/Wake Pin  = 12 (18)
   Shift/Wake Pin  = 33 (13)
   NS8002 Mute pin = 13 (20)

   I2S:

   Microphone:
   I2S_WS 15 (21)
   I2S_SD 32 (12)
   I2S_SCK 14 (17)
   L/R = GND (GND = Left)

   DAC = 25 (14)
   Tone Output = 26 (15)

   I2C:

   I2C_0:
   #define SDA_0 21
   #define SCL_0 22
   TwoWire I2C_0 = TwoWire(0);
   I2C_1:
   #define SDA_1 18
   #define SCL_1 19
   TwoWire I2C_1 = TwoWire(1);

   UART:
   SoftwareSerial SoftSerial(##, ##);        // #=any pins
      SoftSerial.begin(115200);              // void setup() entry
   Serial0: RX0 on GPIO3, TX0 on GPIO1
   Serial1: RX1 on GPIO9, TX1 on GPIO10 (Reserved for Integrated Flash)
   Serial2: RX2 on GPIO16, TX2 on GPIO17 ()


   SPI:
   The SPI(2) pins appear to correspond with VSPI.
   I would have thought SPI(1) pins correspond with HSPI but they don't.

   Native SPI pins are (clk, mosi, miso, cs):
   SPI1: 6, 8, 7, 11
   HSPI: 14,13,12,15
   VSPI: 18,23,19, 5
   
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

// Deffinitions:

//TouchPins:                                        dev cable colour:
#define touchPin T5   // GPIO12   Wake On Touch       Yellow (no plastic sleve)
#define touchPin T2   // GPIO2    TouchValue1/SHIFT   Yellow
#define touchPin T7   // GPIO27   TouchValue2         WHITE
#define touchPin T8   // GPIO33   TouchValue3         ORANGE

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

//On-Demand Config AP:
//define your default Savable Values here. if there are different values in config.json, they are overwritten.
//These appear in the On-Demand Config AP as editable text fields.
char mqtt_server[40];
char mqtt_port[6] = "8080";
char pubTopic[] = "Badges";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//BadgeID:
uint32_t chipId = 0;      //variable to store the BadgeID

void badgeid() {
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.print("BadgeID: ");
  Serial.println(chipId);
}

void callback() {}

void lightSleep() {
  Serial.println("Sleeping Now..");
  delay(100);
  touchAttachInterrupt(12, callback, threshold);        // WAKE on Touch
  touchAttachInterrupt(33, callback, threshold);        // WAKE on Touch
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_timer_wakeup(20 * 60 * 100000);      // 2 = roughly 22 seconds? 10 = 1 minute?
  touch_counter = 0;
  wake_state = 0;
  esp_light_sleep_start( );
}

void wake_from_touch() {

  if ((ESP_SLEEP_WAKEUP_TOUCHPAD) && (touch_counter == 0) && (wake_state == 0)) {
    Serial.println("Wake from Touch");
    wake_state = 1;
    touch_counter = 1;
  }
}

void wake_from_timer() {

  wake_state = 2;
  Serial.println("Timer Wake");
  // Check for Updates from MQTT server here..

}

void combadgeap()
{
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  if (!wifiManager.startConfigPortal("ComBadge_Setup", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Connection Established...");
  badgeid();
  Serial.print("CONNECTED TO: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  Serial.print("SIGNAL: ");
  Serial.print(WiFi.RSSI());

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());

    //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    //end save
  }
  
}

void setup() {
  Serial.begin(115200);   //Serial on usb
  Wire.begin();  //I2C_0.begin(SDA=21 , SCL=22 , 100000 );

  pinMode(ns8002_mute, OUTPUT);         // Mute Pin for Amp
  pinMode(Tones, OUTPUT);               //PWM Tone Output to Amp
  pinMode(12, INPUT_PULLDOWN);          // Touch Wake Pin (MAIN)
  pinMode(33, INPUT_PULLDOWN);          // Touch Wake Pin (SHIFT)
  digitalWrite(ns8002_mute, LOW);
  cap.begin(0x5A);                        //       I2C Address: 0x5A
  cap.setThreshholds(12, 6);
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
#endif

          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
/*
  // Boot Chime
  tone(Tones, 1076);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(20);                   // Waits some time to turn off
  tone(Tones, 1276);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(20);                   // Waits some time to turn off
  tone(Tones, 1476);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(100);                   // Waits some time to turn off
  // Mute Audio Outputs
  dacWrite(DAC1, 0);
  tone(Tones, 0);
  noTone(Tones);
*/
  
  badgeid();


}

void loop() {
  unsigned long currentTime = millis();
  currtouched = cap.touched();        //Call first thing to read from MPR121
  touchValue0 = touchRead(T5);    // Main Touch Wake
  touchValue1 = touchRead(T2);    // SHIFT Touch Wake
  touchValue2 = touchRead(T7);
  touchValue3 = touchRead(T8);    

  //DAC output on pin25
  //int Value = 0; //255=3.3V, 128=1.65V
  //dacWrite(DAC1, Value);

  //Serial.println(cap.touched());          // debug touch sensor
   
  
// Main Functions:

  
  if (touch_counter == 1) {       // Combadge Awake and Listening

    // Active Command Section
    //...
    //Serial.println("Listening...");
    // doop-deep
    delay(20);
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off
    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    //  Record upto 3 seconds from I2S Microphone and send to MQTT server:
  }

  
  if (touch_counter == 2) {       // Deacticate ComBadge

    tone(Tones, 1576);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    touch_counter = 0;
    //delay(20);
  }

  if (touch_counter > 3) {      // Over-Tap Protection
    touch_counter = 0;
  }

  // Internal Touch Pins:
  
  // Internal Touch 0 / Main Touch Pad
  if (touchValue0 < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 0 (IT5) Touched");    
    // doop-deep
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    
    touch_counter++;
  }
  
  // Internal Touch 1
  if (touchValue1 < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 1 (IT2) Touched");
  }
  
  // Internal Touch 2
  if (touchValue2 < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 2 (IT7) Touched");
  }
  



  // MPR121 Touch Pins:
 
  if (cap.touched() & (1 << 0))
  {
    cycle_counter = 0;
    Serial.println("T0 Touched");
    //Tap
    // doop-deep
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    touch_counter++;
  }

  
  if (cap.touched() & (1 << 5))
  {
    
  }
  
  if (cap.touched() & (1 << 6))
  {
    
  }
  
  if (cap.touched() & (1 << 7))
  {
    
  }
  
  if (cap.touched() & (1 << 8))
  {
    
  }

// Multi-Function Key Combinations with touchValue3 being SHIFT

  if ((touchValue3 < threshold) && (cap.touched() & (1 << 2)))    // change to 6 when finished dev Top - On-Demand Config AP
  {
    cycle_counter = 0;
    Serial.println("Shift TOP Touched!");
    combadgeap();
  }

  if ((touchValue3 < threshold) && (cap.touched() & (1 << 7)))    // Middle - Mute
  {
    cycle_counter = 0;
    Serial.println("Shift MID Touched");
  }

  if ((touchValue3 < threshold) && (cap.touched() & (1 << 8)))    // Bottom - 
  {
    cycle_counter = 0;
    Serial.println("Shift BOTTOM Touched");
  }





  //Wake from Sleep Reason and Reaction:
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER : wake_from_timer(); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : wake_from_touch(); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
  }

  lasttouched = currtouched;    //Last call for Capacitive Touch Sensor
  delay(200);
  cycle_counter++;
  //Serial.print("Cycle Counter:");
  //Serial.println(cycle_counter);
  if (cycle_counter > 120) {       //20 = 5 seconds of awake time, 40 = 10 seconds, 120 = 30 seconds, 200 = 50 seconds
    cycle_counter = 0;
    lightSleep();
  }
}

//EOF End-Of-File
