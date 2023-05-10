/* ComBadge mk1 Arduino C++

  last edit 23/04/2023

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
   OEP3W/NS8002 DAC amp               -  DONE
                                      -
   Power Supply                       -  DRAW ~ 6mA/hour in sleep... 150mA Peak @ Boot... Wifi UNTESTED... MQTT Transmission & Recception UNTESTED...
   INMP441 I2S Mic                    -  DONE
   OTA Update                         -
   Chirp on Tap                       -  DONE
   WIFI Manager                       -  DONE


   Pin Definitions = GPIO-Pin, (Board-Pin), (SMD Pins in EasyEDA are different, refer to paper-chart pencil numbers):

   Touch/Wake Pin = 14 (17)
   NS8002 Mute pin = 13 (20)

   I2S:

   Microphone:
   sck = 14 (17)
   ws  = 15 (21)
   sd  = 32 (12)
   L/R = GND (GND = Left)

   DAC = 25 (14)
   Tone Output = 27 (16)

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

   Tone Generation:

    // Sounds the Tones at the frequency relative to the note C in Hz
    tone(Tones,261);
    // Waits some time to turn off
    delay(200);
    //Turns the Tones off
    noTone(Tones);
    // Sounds the Tones at the frequency relative to the note D in Hz
    tone(Tones,293);
    delay(200);
    noTone(Tones);
    // Sounds the Tones at the frequency relative to the note E in Hz
    tone(Tones,329);
    delay(200);
    noTone(Tones);
    // Sounds the Tones at the frequency relative to the note F in Hz
    tone(Tones,349);
    delay(200);
    noTone(Tones);
    // Sounds the Tones at the frequency relative to the note G in Hz
    tone(Tones,392);
    delay(200);
    noTone(Tones);
*/

// Libraries:
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>            // Capacitive Touch Sensor @ 0x5A
#include <Arduino.h>
#include <SoftwareSerial.h>

#include <AudioConfig.h>
#include <AudioLogger.h>
#include <AudioTools.h>

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

// Deffinitions:

//TouchPins:
#define touchPin T2   // GPIO2    TouchValue1
#define touchPin T5   // GPIO12   TouchValue2
#define touchPin T7   // GPIO27   TouchValue3

//I2S:
#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14

// Use ESP32 I2S Processor 0
#define I2S_PORT I2S_NUM_0
#define bufferLen 64
int16_t sBuffer[bufferLen];

//DAC:
#define DAC1 25
#define ns8002_mute 13
#define Tones 26

//I2C:
// SDA 21
// SCL 22

//UART:
// Serial0: RX0 on GPIO3, TX0 on GPIO1
//SPI:

// Initillizations:

//Touch Settings (Internal):
const int threshold = 40;
int touchValue1;
int touchValue2;
int touchValue3;

//Touch Settings (MPR121):
Adafruit_MPR121 cap = Adafruit_MPR121();      // Capacitive Sensor
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

int touch_counter = 0;
int cycle_counter = 0;
int wake_state = 0;




void callback() {}

void lightSleep() {
  Serial.println("Sleeping Now..");
  delay(100);
  touch_counter = 0;
  wake_state = 0;
  touchAttachInterrupt(14, callback, threshold);
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_timer_wakeup(20 * 60 * 100000);      // 2 = roughly 22 seconds? 10 = 1 minute?
  esp_light_sleep_start( );
}

void wake_from_touch() {

  if ((ESP_SLEEP_WAKEUP_TOUCHPAD) && (touch_counter == 0) && (wake_state == 0)) {
    Serial.println("Wake from Touch");
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
    wake_state = 1;
    touch_counter = 1;
  }
}

void wake_from_timer() {

  wake_state = 2;
  Serial.println("Timer Wake");
  // Check for Updates from MQTT server here..

}


void setup() {
  Serial.begin(9600);   //Serial on usb
  Wire.begin();  //I2C_0.begin(SDA=21 , SCL=22 , 100000 );

  pinMode(ns8002_mute, OUTPUT);         // Mute Pin for Amp
  pinMode(Tones, OUTPUT);               //PWM Tone Output to Amp
  pinMode(14, INPUT_PULLDOWN);
  digitalWrite(ns8002_mute, LOW);
  cap.begin(0x5A);                        //       I2C Address: 0x5A
  cap.setThreshholds(12, 6);

  // Boot Chime
  tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(20);                   // Waits some time to turn off
  tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(20);                   // Waits some time to turn off
  tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(200);                   // Waits some time to turn off
  // Mute Audio Outputs
  dacWrite(DAC1, 0);
  tone(Tones, 0);
  noTone(Tones);

}

void loop() {
  unsigned long currentTime = millis();
  currtouched = cap.touched();        //Call first thing to read from MPR121
  touchValue1 = touchRead(T2);
  touchValue2 = touchRead(T5);
  touchValue3 = touchRead(T7);

  //DAC output on pin25
  //int Value = 0; //255=3.3V, 128=1.65V
  //dacWrite(DAC1, Value);

  //Serial.println(cap.touched());          // debug touch sensor

  if (touchValue1 < threshold) {      // Internal Touch 1 - SCAN MODE TOGGLE
    Serial.println("IT1 Touched");
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

    if (!wifiManager.startConfigPortal("ComBadge_Setup", "password")) {
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

  if (touchValue2 < threshold) {      // Internal Touch 1 - SCAN MODE TOGGLE
    Serial.println("IT2 Touched");
    //beep();
  }

  if (touchValue3 < threshold) {      // Internal Touch 1 - SCAN MODE TOGGLE
    Serial.println("IT3 Touched");
    //beep();
  }

  if (cap.touched() & (1 << 1))                                 //First-Tap
  {
    //Tap
    // doop-deep
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off
    touch_counter++;
  }


  if (touch_counter == 1) {       // Combadge Awake and Listening

    // Active Command Section
    cycle_counter = 0;
    Serial.println("Listening...");

    //  Record upto 3 seconds from I2S Microphone and send to MQTT server

  }

  if (touch_counter == 2) {       // Deacticate ComBadge

    //dom -tone
    touch_counter = 0;
    delay(20);
    tone(Tones, 1576);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
  }

  if (touch_counter > 3) {
    touch_counter = 0;
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
  if (cycle_counter > 20) {       //20 = 5 seconds of awake time
    cycle_counter = 0;
    lightSleep();
  }
}

//EOF End-Of-File
