
/* ComBadge mk1 Arduino C++

  Linux Compile:
  
  Default (1.5Mb SPIFFS):
  Sketch uses 865817 bytes (66%) of program storage space. Maximum is 1,310,720 bytes.
  Global variables use 45224 bytes (13%) of dynamic memory, leaving 282456 bytes for local variables. Maximum is 327680 bytes.

  HUGE APP (1Mb SPIFFS):
  Sketch uses 866541 bytes (27%) of program storage space. Maximum is 3,145,728 bytes.
  Global variables use 45232 bytes (13%) of dynamic memory, leaving 282448 bytes for local variables. Maximum is 327680 bytes.


   FDTI: COM14
   DevBoard: COM15

   Settings for Arduino IDE:
   BOARD:           ESP32-WROOM-(32)DA
   UPLOAD SPEED:    921600
   CPU FREQ:        240mhz
   FLASH SIZE:      4mb(Default)
   FLASH MODE:      QIO
   PARTITION:       HUGE SPIFFS (NO OTA)/Default SPIFFS with OTA & Files



  TO DO:

   ESP32-WROOM-32E - 2Mb Flash    -   Code Checklist:
   BadgeID                            -  DONE
   OEP3W/NS8002 DAC amp               -  DONE
   Power Supply                       -  DRAW of esp32 @ 3.3v: 6-15mAh/hour in sleep... 150mAh Peak @ Boot... Wifi > 130mah... Wake from Timer = 96mAh... MQTT Transmission & Recception UNTESTED...
                                      -  Draw of Transmitter @ 5v: 285~300ma with wifi connection, 285>450ma when playing sound, 350ma wifi-AP active,  250ma when idle
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

//EOF End-Of-File
