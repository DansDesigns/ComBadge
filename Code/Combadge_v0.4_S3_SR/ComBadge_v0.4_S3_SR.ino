/*  ESP32 ComBadge (Voxari) V0.3
    Compatible with ESP32-S3 ONLY

  ESP32-S3 Features:
  =====================================
  Deep Sleep with Touch Wake Up
  WiFi File Browser

  =====================================
  debug & debugln Defaults to USB
  Serial0.print sends to TX,RX UART (4g & Lora)
  ================================================================================
  /!\\ WiFi-File Browser is OPEN-AS on your lcoal network and
                                      anyone can delete/modify/see everything /!\\
  ================================================================================
  Only RTC IO can be used as a source for external wake.
  They are pins: 0,2,4,12-15,25-27,32-39.

  MTCH6102 Interrupt goes to Pin 2
  ================================================================================
*/

#include "driver/rtc_io.h"
#include <FS.h>
#include <SD.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <SPI.h>
#include <Wire.h>

#include "ESP_I2S.h"
#include "ESP_SR.h"
#include "Wav.h"

#include <WiFi.h>
#include "cert.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

#include <ESPFMfGK.h>  // https://github.com/holgerlembke/ESPFMfGK
// have a look at this concept to keep your private data safe!
//#include <privatedata.h>  // https://github.com/holgerlembke/privatedata

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#include <husarnet.h>  // https://github.com/husarnet/husarnet-esp32-arduino
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> //Documents\Arduino\libraries\WiFiManager
#include <Arduino.h>


// ==============DEBUG==============
#define DEBUG 1     // 1 = use print statements, 0 = dont use print statements

#if DEBUG == 1
#define debug(x)      Serial.print(x)
#define debugf(x,y)   Serial.printf(x,y)
#define debugln(x)    Serial.println(x)
#else
#define debug(x)
#define debugf(x,y)
#define debugln(x)
#endif
// ==============END DEBUG==============

//=========== Color definitions ===========
#define OFF        0,0,0
#define WHITE      10,10,10
#define GREY       6,6,6
#define DARKGREY   5,5,5
#define TURQUOISE  0,10,10
#define PINK       10,4,5
#define PINKRED    10,4,0
#define OLIVE      4,4,0
#define PURPLE     4,0,4
#define AZURE      0,4,10
#define ORANGE     10,6,0
#define YELLOW     10,10,0
#define GREEN      0,10,0
#define BLUE       0,0,10
#define BLUEVIOLET 8,0,8
#define RED        10,0,0
#define AMBER      10,8,6
#define FULL       255,255,255

//=========== LED Basic Control ===========
#define LED_1_(x,y,z)   rgbLedWrite(RGB_BUILTIN,x,y,z)
#define LED_2_(x,y,z)   rgbLedWrite(21,x,y,z)
//
#define LED_1_OFF      rgbLedWrite(RGB_BUILTIN,OFF)
#define LED_2_OFF      rgbLedWrite(21,OFF);
#define LED_1_OK       rgbLedWrite(RGB_BUILTIN,GREEN);
#define LED_2_OK       rgbLedWrite(21,GREEN);
//=========== LED STATUS CODES ===========
// LED_1 =  GPIO48
// LED_2 =  GPIO21


// LED_1 + LED_2 - BOOT/Battery Status Indicator:
#define LED_1_BOOT         rgbLedWrite(RGB_BUILTIN,BLUEVIOLET);
#define LED_2_BOOT         rgbLedWrite(21,AZURE);
#define LED_1_BATTERY100   rgbLedWrite(RGB_BUILTIN,GREEN);
#define LED_2_BATTERY100   rgbLedWrite(21,GREEN);
#define LED_1_BATTERY60    rgbLedWrite(RGB_BUILTIN,ORANGE);
#define LED_2_BATTERY60    rgbLedWrite(21,ORANGE);
#define LED_1_BATTERY20    rgbLedWrite(RGB_BUILTIN,RED);
#define LED_2_BATTERY20    rgbLedWrite(21,RED);
#define LED_1_BATTERY10    rgbLedWrite(RGB_BUILTIN,RED); delay(300); rgbLedWrite(RGB_BUILTIN,OFF); delay(300);rgbLedWrite(RGB_BUILTIN,RED); delay(300); rgbLedWrite(RGB_BUILTIN,OFF);
#define LED_2_BATTERY10    rgbLedWrite(21,RED); delay(300); rgbLedWrite(RGB_BUILTIN,OFF); delay(300);rgbLedWrite(RGB_BUILTIN,RED); delay(300); rgbLedWrite(RGB_BUILTIN,OFF);


// LED 1 - Connection Indicator:
#define LED_1_NOCON            rgbLedWrite(RGB_BUILTIN,RED)
#define LED_1_CONNECTING       rgbLedWrite(RGB_BUILTIN,ORANGE)
#define LED_1_WIFI_CONNECTED   rgbLedWrite(RGB_BUILTIN,BLUE)
#define LED_1_4G_CONNECTED     rgbLedWrite(RGB_BUILTIN,GREEN)
#define LED_1_MESH_CONNECTED   rgbLedWrite(RGB_BUILTIN,BLUEVIOLET)
#define LED_1_ERROR            rgbLedWrite(RGB_BUILTIN,RED)

// LED 2 - State Indicator:
#define LED_2_STANDBY          rgbLedWrite(21,GREY);
#define LED_2_LISTENING        rgbLedWrite(21,GREEN);
#define LED_1_RECORDING        rgbLedWrite(21,BLUE)
#define LED_2_INCOMING         rgbLedWrite(21,AMBER);
#define LED_2_OUTGOING         rgbLedWrite(21,GREEN);
#define LED_2_NOTIFICATION     rgbLedWrite(21,RED);




// ==================== Main Definitions ====================
bool srListenRequested = false;
char Mode = 'z';      // 'z' = main/idle mode
bool call_phone = false;
bool call_badge = false;
bool wifiReady = false;

#define WAKEUP_GPIO GPIO_NUM_2   // EXT0 wake from MTCH6102 interrupt

// ==================== BadgeID ====================
// Derived from the chip's unique eFuse MAC at startup (inline in setup, no task needed)
uint64_t chipId  = 0;
String   BadgeID = "";

void initBadgeID() {
  for (int i = 0; i < 17; i += 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xFF) << i;
  }
  uint32_t low  = (uint32_t)(chipId & 0xFFFFFFFF);
  uint32_t high = (uint32_t)(chipId >> 32);
  // Produces a hex string like "a1b2c3d4e5f6" — unique per chip
  BadgeID = String(high, HEX) + String(low, HEX);
  debugln("============================================================");
  debug("BadgeID: ");
  debugln(BadgeID);
  debugln("=======================^^BadgeID^^==========================");
}

Preferences preferences;

// ==================== Husarnet VPN ====================
// Husarnet provides stable peer-to-peer addressing across WiFi, LTE, and
// WiFi HaLow. Badges can call each other from anywhere with internet.
// BadgeID is used as the hostname so every badge has a fixed, routable identity.
//
// Your Husarnet network join code is NO LONGER hardcoded here.
// It is stored as an AES-128 encrypted file on the SD card (/husarnet.key).
// The decryption key lives in NVS (Preferences), provisioned at manufacture.
// See husarnet_key.ino for full details.
//
// To provision a new badge:
//   1. Flash firmware
//   2. Open Badge NVS Deployer
//   3. paste join code
//   4. Done.
//
//=============================================================================================
// Your Husarnet network join code. !-keep this private, treat like a password-!
static String husarnetJoinCode = "";   // loaded at boot by husarnetKeySetup() + loadJoinCode()
//=============================================================================================
HusarnetClient husarnet;
volatile bool husarnet_connected = false;  // volatile: written by task, read by main core

// new version from SD:
void husarnet_task(void* pvParams) {
  // Join code was loaded from encrypted SD file into husarnetJoinCode at boot
  if (husarnetJoinCode.isEmpty()) {
    debugln("Husarnet: no join code available - check SD card has /husarnet.key and badge is provisioned.");
    vTaskDelete(NULL);
  }

  // Hostname = BadgeID so every badge is reachable by its unique chip ID
  husarnet.join(BadgeID.c_str(), husarnetJoinCode.c_str());

  unsigned long startTime = millis();
  while (!husarnet.isJoined()) {
    if (millis() - startTime > 30000) {
      debugln("Husarnet: timed out waiting to join.");
      vTaskDelete(NULL);
    }
    debugln("Husarnet: waiting to join...");
    // Note: do NOT use delay() inside a FreeRTOS task, use vTaskDelay()
    // Also avoid LED_2_WAIT here as it contains delay() calls
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  husarnet_connected = true;
  debug("Husarnet joined. IPv6: ");
  String ip = String(husarnet.getIpAddress().c_str());
  saveHusarnetIP(ip);
  debugln(ip);
  vTaskDelete(NULL);
}


void startHusarnet() {
  if (WiFi.status() != WL_CONNECTED) {
    debugln("Husarnet: skipping - WiFi not connected.");
    return;
  }
  debugln("Husarnet: starting task on Core 0...");
  xTaskCreatePinnedToCore(
    husarnet_task,   // task function
    "husarnet",      // name
    16384,           // stack size- must be large, default causes stack overflow on S3
    NULL,            // parameters
    1,               // priority
    NULL,            // handle
    0                // Core 0 - keep away from ESP-SR which runs on Core 1
  );
}

// ==================== WiFi Setup ====================
const word filemanagerport = 81;
ESPFMfGK filemgr(filemanagerport);
WiFiManager wifiManager;
NetworkServer server(80);

#define WIFI_TIMEOUT_MS 5000
#define LENGTH(x)       (strlen(x) + 1)
#define EEPROM_SIZE     200
#define WiFi_rst        0         // Boot button - hold 3s to clear WiFi creds

unsigned long rst_millis;
unsigned long sleep_millis;

String esid  = "";   // loaded from EEPROM at boot - do NOT hardcode credentials here
String epass = "";   // loaded from EEPROM at boot - use SmartConfig to set on first run

const char* textURL = "http://api.ipify.org/";

bool shouldSaveConfig  = false;
bool filemanager_active = false;

// ==================== Firmware Version ====================
String FirmwareVer = "0.3";
#define URL_fw_Version "https://raw.githubusercontent.com/DansDesigns/ComBadge/main/firmware/version.txt"
#define URL_fw_Bin     "https://raw.githubusercontent.com/DansDesigns/ComBadge/main/firmware/fw.bin"

// ==================== MTCH6102 ====================
#define ADDR                  0x25
#define MTCH6102_Interupt_Pin 2    // 4.7K resistor to 3V3

unsigned char readRegister(unsigned char addr_reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(addr_reg);
  Wire.endTransmission();
  Wire.requestFrom(ADDR, 1);
  while (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

// ==================== SD Card ====================
const uint8_t mosi = 12;  // D1
const uint8_t sck  = 13;  // CLK
const uint8_t cs   = 14;  // CS
const uint8_t miso = 47;  // D0
// VSS = GND, VCC = 3V3

// ==================== I2S ====================
#define I2S_PIN_BCK  5   // SCK  - INMP441 / MAX98357 CLK
#define I2S_PIN_WS   4   // WS   - INMP441 / MAX98357 LRC
#define I2S_PIN_DIN  7   // SD   - INMP441 (mic output)
#define I2S_PIN_DOUT 6   // changed from 6 to 16 to test errors on breadboard    - MAX98357 DIN (speaker input)

I2SClass i2s;
SPIClass sdSPI(FSPI);

#define RECORD_SECONDS 5
const char filename[] = "/recording.wav";
const int  headerSize  = 44;
byte       header[headerSize];
File       file;

// ==================== ESP-SR Commands ====================
// Fixed commands — IDs are stable, contact commands are appended at boot.
enum {
  SR_CMD_ZERO,
  SR_CMD_ONE,
  SR_CMD_TWO,
  SR_CMD_THREE,
  SR_CMD_FOUR,
  SR_CMD_FIVE,
  SR_CMD_SIX,
  SR_CMD_SEVEN,
  SR_CMD_EIGHT,
  SR_CMD_NINE,
  SR_CMD_CALL_BADGE,       // 10 — generic "call badge" (falls back to first contact)
  SR_CMD_CALL_PHONE,       // 11
  SR_CMD_BEGIN_RECORDING,  // 12
  SR_CMD_OPEN_CONFIG,      // 13
  SR_CMD_CLOSE_CONFIG,     // 14
  SR_CMD_OPEN_FILES,       // 15
  SR_CMD_CLOSE_FILES,      // 16
  SR_CMD_UPDATE_CONTACTS,  // 17
  SR_CMD_UPDATE_WIFI,      // 18
  SR_CMD_ENABLE_WIFI,      // 19
  SR_CMD_DISABLE_WIFI,     // 20
  SR_CMD_ENABLE_SIM,       // 21
  SR_CMD_DISABLE_SIM,      // 22
  SR_CMD_FIXED_COUNT       // 23 — marks end of fixed block; contact IDs start here
};

// SR_CMD_CONTACT_BASE is the first dynamic contact command ID (== SR_CMD_FIXED_COUNT).
// Any command_id >= SR_CMD_CONTACT_BASE maps to contacts[command_id - SR_CMD_CONTACT_BASE].
#define SR_CMD_CONTACT_BASE  SR_CMD_FIXED_COUNT

// Fixed portion — never changes, built at compile time.
static const sr_cmd_t sr_fixed_commands[] = {
  {  0, "Zero",             "ZgRb"             },
  {  1, "One",              "WcN"              },
  {  2, "Two",              "To"               },
  {  3, "Three",            "vRm"              },
  {  4, "Four",             "FeR"              },
  {  5, "Five",             "FiV"              },
  {  6, "Six",              "SgKS"             },
  {  7, "Seven",            "SfVcN"            },
  {  8, "Eight",            "dT"               },
  {  9, "Nine",             "NiN"              },
  { 10, "Call Badge",       "KeL Baq"          },
  { 11, "Call Phone",       "KeL FbN"          },
  { 12, "Begin Recording",  "BgGgN RcKeRDgl"   },
  { 13, "Open Config",      "bPcN KnNFgG"      },
  { 14, "Close Config",     "KLbS KnNFgG"      },
  { 15, "Open Files",       "bPcN FiLZ"        },
  { 16, "Close Files",      "KLbS FiLZ"        },
  { 17, "Update Contacts",  "cPDdT KnNTaKTS"   },
  { 18, "Update WiFi",      "cPDdT WiFi"       },
  { 19, "Enable Wifi",      "fNdBcL WiFi"      },
  { 20, "Disable Wifi",     "DgSdBcL WiFi"     },
  { 21, "Enable Sim",       "fNdBcL SgM"       },
  { 22, "Disable Sim",      "DgSdBcL SgM"      },
};
#define SR_FIXED_COUNT  (sizeof(sr_fixed_commands) / sizeof(sr_cmd_t))

// Dynamic command table — allocated in buildSrCommands(), freed on rebuild.
// sr_commands_dynamic points to the heap buffer passed to ESP_SR.begin().
sr_cmd_t* sr_commands_dynamic = nullptr;
int       sr_commands_count    = 0;

// nameToPhoneme() and buildSrCommands() live in ca_contacts.ino
// where the Contact struct is fully defined.

// ==================== Wakeup Reason ====================
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     debugln("Wakeup: EXT0 RTC_IO");     break;
    case ESP_SLEEP_WAKEUP_EXT1:     debugln("Wakeup: EXT1 RTC_CNTL");   break;
    case ESP_SLEEP_WAKEUP_TIMER:    debugln("Wakeup: Timer");            break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: debugln("Wakeup: Touchpad");         break;
    case ESP_SLEEP_WAKEUP_ULP:      debugln("Wakeup: ULP program");      break;
    default: Serial.printf("Wakeup: not deep sleep (%d)\n", wakeup_reason); break;
  }
}

void saveConfigCallback() {
  debugln("Should save config");
  shouldSaveConfig = true;
}

//EOF
