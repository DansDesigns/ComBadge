/*  ESP32 ComBadge V0.3 Changelog
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  06/04/2026
  =================
  Sketch uses 2163242 bytes (68%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 74252 bytes (22%) of dynamic memory, leaving 253428 bytes for local variables. Maximum is 327680 bytes.





  05/04/2026-2
  =================

  ComBadge_v0_3_S3_SR.ino:
  The old sr_commands[] was a static const array — baked into flash, impossible to extend at runtime. Replaced with:

  sr_fixed_commands[] — the same 23 fixed commands, still compile-time const
  sr_commands_dynamic — a heap-allocated sr_cmd_t* built at boot
  buildSrCommands() — called after loadContacts(), allocates the full table as [fixed 23] + [one "Call <n>" per contact]. Assigns IDs starting at SR_CMD_CONTACT_BASE (= 23)
  nameToPhoneme() — converts a plain name like "Support" to an ESP-SR phoneme string automatically using the MultiNet phoneme rules. If the auto-result sounds wrong you can override it per-contact in contacts.json with a "phoneme" field

  zsetup.ino:
  ESP_SR.begin() is now deferred — moved to after loadContacts() and buildSrCommands(), so the recogniser is initialised with the full command table including all contact names
  Passes sr_commands_dynamic and sr_commands_count instead of the old static array

  onsrevent.ino:
  Added a check before the switch: if command_id >= SR_CMD_CONTACT_BASE, it maps directly to contacts[id - SR_CMD_CONTACT_BASE] and calls initiateCall() immediately — no intermediate flag, no call_badge detour.

  call_manager.ino:
  Removed the old TODO: call contacts[0] hack. The call_badge flag is now only a fallback for the generic "Call Badge" utterance — with one contact it calls them, with multiple it flashes red and tells you to be specific.

  ca_contacts.ino:
  Added phoneme[48] field to the Contact struct
  loadContacts() reads an optional "phoneme" key from JSON
  saveContacts() writes it back only if non-empty




  05/04/2026
  ==============
  added persistant husarnet IPv6 address to NVS


  Sketch uses 2157474 bytes (68%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 73284 bytes (22%) of dynamic memory, leaving 254396 bytes for local variables. Maximum is 327680 bytes.



  28/03/26
  ============
  rewrote led_2 control to use same method as built-in rgb on pin 48:
  rgbLedWrite(21, <COLOUR>);

  added colour codes for easy changing of colours & human readable code:

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
  #define SKYBLUE    135,206,235
  #define BLUE       0,0,10
  #define BLUEVIOLET 8,5,10
  #define AQUAMARINE 127,255,212
  #define RED        255,53,0
  #define HONEY      255,204,153
  #define BUTTER     255,238,204
  #define OCTSUNSET  255,68,0
  #define FULL       255,255,255





  25/03/26
  ============

  fixed audio issues with encoded .h files for tng chirp..
  removed LED_2_ referencces completely.. need to look at adafruit_neopixel library:
  line 3222:
  espShow(pin, pixels, numBytes, is800KHz);

  updated gesture list:

      case 0x10: debugln("Single click");         LED_1_READY; playSoundAsync(SOUND_CHIRP); srListenRequested = true; break;
      case 0x11: debugln("Click and hold");       ESP.restart(); break; // cancel
      case 0x20: debugln("Double click");         break; // ???
      case 0x31: debugln("Down swipe");           break; // recieve contact chirps
      case 0x32: debugln("Down swipe and hold");  break; // volume down
      case 0x41: debugln("Right swipe");          break; //
      case 0x42: debugln("Right swipe and hold"); break; // battery level check?
      case 0x51: debugln("Up swipe");             break; // send contact chirp
      case 0x52: debugln("Up swipe and hold");    break; // volume up
      case 0x61: debugln("Left swipe");           break; //
      case 0x62: debugln("Left swipe and hold");  smartConfig(); break; // configAP?


  Sketch uses 2156622 bytes (68%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 73308 bytes (22%) of dynamic memory, leaving 254372 bytes for local variables. Maximum is 327680 bytes.


  23/03/26 -
  =======================

  updated sounds.no & setup to make ESP_SR load first on I2S_num_0, then load I2S out on I2s_num_1

  LED_2 still needs fixing, neopixel library causes restarts..

  current code is slow and hangs sometimes after joining husarnet..

  crashes and restarts sometimes when single tap & playing (star trek) chirp.h:
  Backtrace: 0x40376959:0x3fcbe500 0x40380ec1:0x3fcbe520 0x40387ce6:0x3fcbe540 0x40384102:0x3fcbe680 0x4038170f:0x3fcbe6a0 0x42024f80:0x3fcbe6e0 0x42025177:0x3fcbe700 0x42008877:0x3fcbe720 0x4202b4f4:0x3fcbe760 0x40381bf6:0x3fcbe780



  Sketch uses 2157374 bytes (68%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 73308 bytes (22%) of dynamic memory, leaving 254372 bytes for local variables. Maximum is 327680 bytes.



  19/03/2026 - update 6
  ========================
  added python program to generate encrypted husarnet.key and burn decryption key into NVStorage
  Husarnet passcode moved to encrypted file on SD card with decryption key stored in NVS

  updated firmware_update, setup, loop, main file
  added husarnet_key.ino to manage the NVS key

  removed smartconfig & replaced with captive wifi portal with info page

  added 3 init tries for SD

  updated LED_2 to have a wifiReady flag

  neopixel 2 not working..


  Sketch uses 2158186 bytes (68%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 73300 bytes (22%) of dynamic memory, leaving 254380 bytes for local variables. Maximum is 327680 bytes.



  18/03/2026 - 2
  ========================================

  added sound playback function for embedded .h files (converted .wav)
  added chirp sound
  added ring sound
  updated arduino-esp32-core to version 3.2.0
  added speaker test in setup (speakertest.ino needs updating)

  ==========================ISSUES==============================
  ISSUE: problem with smartconfig, replace with captive web portal (lcars style?)
  adjusted i2srecording - still has issues..


  Sketch uses 2108726 bytes (67%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 72788 bytes (22%) of dynamic memory, leaving 254892 bytes for local variables. Maximum is 327680 bytes.



  18/03/2026
  ========================================
  updated husarnet connection, removed redundant wifi connection clause in keepwifi.ino

  re-wrote mqtt_presense.ino to remove mqtt entirely and use UDP packets with Presence Broadcast


  Sketch uses 2053372 bytes (65%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 71156 bytes (21%) of dynamic memory, leaving 256524 bytes for local variables. Maximum is 327680 bytes.



  17/03/2026 - update 5 (not fully installed, see update 6)
  ========================


  updated contacts, audio_stream, main file, call_functions, call_manager, onsrevent

  renamed contacts to ca_contacts



  Sketch uses 2054544 bytes (65%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 70884 bytes (21%) of dynamic memory, leaving 256796 bytes for local variables. Maximum is 327680 bytes.




  16/03/2026 - update 4
  ========================

   new files:

   contacts.ino - the address book, since everything else depends on being able to resolve a BadgeID to an IP
   mqtt_presence.ino - presence publishing and IP updates
   audio_stream.ino - the actual streaming tasks
   call_manager.ino - wiring it all together with the SR events


   Functions:

   contacts.ino - loads /contacts.json from SD into a contacts[] array on boot. updateContactIP() is called automatically by MQTT when a contact comes online.
   Create your contacts file on the SD card - example entry to get you started with badge 2 when it's built:
   json[{"name":"Badge2","badgeID":"yourchipidhere","ip":""}]

   The IP field starts blank and gets filled in automatically via MQTT the first time badge 2 boots.

   mqtt_presence.ino - fill in MQTT_BROKER with your broker address before flashing. On first boot it publishes your Husarnet IP as a retained message, subscribes to all your contacts' presence topics, and subscribes to your own call topic for incoming rings. Reconnects automatically if the broker drops.

   audio_stream.ino - raw PCM over TCP on port 3000. Both tasks run on Core 0. ESP-SR is suspended during a call and resumed on end. The NetworkServer audioServer is separate from server on port 80 so they don't interfere.

   call_manager.ino - one thing to note: right now "Call Badge" + SR calls the first contact in your list as a placeholder. The TODO comment marks where you'll wire in spoken name recognition once you have a second badge to test with. acceptCall() and rejectCall() are ready to be hooked into SR commands - you'll want to add "Accept" and "End Call" to sr_commands[] in the main file.


  new libraries:

  ArduinoJson v6 - contacts.ino - search "ArduinoJson" by Benoit Blanchon
  PubSubClient - mqtt_presence.ino - search "PubSubClient" by Nick O'Leary




  16/03/2026 - update 3
  ========================
  client_connected() never called - added to zzloop.ino. Without this the HTTP server existed but never handled any requests
  husarnet_connected not volatile - fixed. Written by the Husarnet FreeRTOS task on Core 0, read by ip.ino on Core 1 - without volatile the compiler could cache the value and never see it update
  delay() inside husarnet_task - removed. delay() in a FreeRTOS task blocks the scheduler tick for that core. Now uses vTaskDelay() only
  WebServer.h included but unused - removed. NetworkServer is used everywhere instead
  httpCode uninitialised in firmware_update.ino - fixed, initialised to -1
  EEPROM.begin() called twice in configAP - left as-is, harmless on ESP32

  Sketch uses 2031516 bytes (64%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 68812 bytes (20%) of dynamic memory, leaving 258868 bytes for local variables. Maximum is 327680 bytes.


  15/03/2026 - update 2
  ========================

  -ComBadge_v0_2_S3_SR.ino - Husarnet include is back, plus a proper husarnet_task() function with 16KB stack pinned to Core 0, and a startHusarnet() helper that checks WiFi is up before launching it. The hostname is now set to BadgeID rather than the hardcoded "VoxAir-Badge" - so each badge is reachable on the Husarnet network by its unique chip ID. husarnet_connected bool flag lets the rest of the code know when it's safe to use the Husarnet address.

  -zsetup.ino - startHusarnet() called at the end of setup() after keepWifiAlive(). It's non-blocking - the task runs in the background and setup() returns immediately, so boot time isn't held up waiting for VPN join.

  -ip.ino - added getHusarnetIP() which returns the Husarnet IPv6 address as a String, or empty string if not yet joined. This is the address that will go into the MQTT presence publish when we build mqtt_presence.ino - it's the address other badges use to reach you regardless of what network you're on.


  14/03/2026 - Claude re-write
   =============================

   re-write of following:

   ComBadge_v0_2_S3_SR.ino - globals and definitions
   Wav.cpp / Wav.h - fix sample rate
   i2srecord.ino - fix double header
   call_functions.ino - fix the three bugs
   keepwifi.ino - deduplicate
   zsetup.ino - fix SD init
   zzloop.ino - reduce delay
   listen_server.ino - replace placeholder



   full changelog:

  -ComBadge_v0_2_S3_SR.ino - removed husarnet.h include and HusarnetClient (was crashing, commented out everywhere anyway), removed LittleFS include (never used), removed global wanIP (shadowed by local in ip.ino), removed old recording dead vars (numCommunicationData etc.), removed empty callback(), fixed BadgeID from integer addition to proper hex string, moved badgeid() task into an inline initBadgeID() function to eliminate the race condition, added #define RECORD_SECONDS 5 as single source of truth.

   -Wav.cpp / Wav.h - corrected sample rate from 44100 Hz to 16000 Hz, updated ByteRate (32000) and BlockAlign (2) to match. Your recordings will now play back at the correct speed.

   -i2srecord.ino - removed the double WAV header bug. i2s.recordWAV() already returns a complete WAV file in the buffer, the old CreateWavHeader + file.write(header) before it was prepending a
    second header. This was silently corrupting every recording.

   -call_functions.ino - fixed calling_badge()/calling_phone() (flags were swapped), fixed number_1() assignment-not-comparison (= → ==), removed the nonsensical WAV header write to call_log.txt.

   -keepwifi.ino - deduplicated the external_ip() call (was being called twice on a fresh connect), added a bool return value so callers can know if WiFi actually came up.

   -zsetup.ino - SD.begin() now called once with a warning log instead of a blocking while loop, initBadgeID() replaces the task launch, removed husarnet remnants.

   -zzloop.ino - delay(100) → delay(10) at the bottom of loop. Keeps things responsive and won't fight the audio streaming tasks when we add them.

   -listen_server.ino - completely replaced the LED-pin-5 placeholder. Now serves GET / → badge info JSON (ID, IP, firmware, RSSI, status) and GET /ping → pong. The GET /stream and POST /call
    endpoints are the natural next additions when we build audio_stream.ino and call_manager.ino.



   ==================================================
   moved focus to development of OSM-Phone for 1 year
   ==================================================

   29/04/2025 - Husarnet VPN Update
   ======================================

   all compiles:
    Sketch uses 2099100 bytes (66%) of program storage space. Maximum is 3145728 bytes.
    Global variables use 69020 bytes (21%) of dynamic memory, leaving 258660 bytes for local variables. Maximum is 327680 bytes.



   26,27/02/2025 - Neopixel, mtch6102, MAX98357 & PCB updates
   ===========================================================
   - added seperate control for 2nd status led,
        - 2nd on adafruit library with 1st led on RGB_BUILTIN
   - added easier control for status leds: LED_#_<FUNCTION> - see top of main file
   - fixed hissing crackling sound during boot
   - added mute function in code
   - fixed mtch6102 switch case
   - replaced SIM7080g with A7683E:
        - Supported Frequency Bands: LTE-FDD B1/B3/B5/B7/B8/B20/B28
        - North America: Bands 2, 4, 12, and 66 are commonly used
        - Europe: Bands 3, 7, and 20 are common
        - Asia-Pacific: Bands 1, 3, and 8 are commonly used
   -



   ######
   ISSUES
   ######
   - wake from EXT0
   - I2S record to file - file empty only 1 second long
   -

  Sketch uses 1563172 bytes (49%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 58012 bytes (17%) of dynamic memory, leaving 269668 bytes for local variables. Maximum is 327680 bytes.




   23,24,25/02/2025 - file browser fix, BadgeID String fix,
   ===================================================
   - fixed wifi file browser
   - added BadgeID uint64_t > String conversion
   - re-write webserver code to check for connections then start streaming


  Sketch uses 1561740 bytes (49%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 58300 bytes (17%) of dynamic memory, leaving 269380 bytes for local variables. Maximum is 327680 bytes.


   22/02/2025 - husarnet & mpr121 removal & EXT0 wake
   ===================================================
   - removed husarnet for now, fails to connect
   - external ip re-worked
   - removed mpr121 as no longer needed
   - removed ESP touch wake
   - added EXT0 wake on pin 2 from mtch6102


   ########
   ISSUES:
   ########
   - not currently waking from light_sleep)

  Sketch uses 1561740 bytes (49%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 58300 bytes (17%) of dynamic memory, leaving 269380 bytes for local variables. Maximum is 327680 bytes.



   18/02/2025 - WIFI UPDATE
   ========================
   - added SmartConfig - Android App to connect wifi
   - fixed ESP32-S3 WiFi connection issues by adding WiFi.setTxPower(WIFI_POWER_8_5dBm); after each WiFi.begin and softAP call
        #for fix, see: https://github.com/espressif/arduino-esp32/issues/6767
   - added wake on ext pin2 (interupt from mtch6102)
   - minimised wifi connection functions


   #######
   ISSUES:
   #######
   - Hursanet VPN crashing on hursanet.join()...
      ** created Github issue in hursanet-arduino-esp32 repo


  Sketch uses 2173468 bytes (69%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 69756 bytes (21%) of dynamic memory, leaving 257924 bytes for local variables. Maximum is 327680 bytes.



   15/02/2025 - multi-task redesign, LTE integration
   ===============================================
   - redesigned multi-tasking so wifi is in main loop & other bits are in their own xtaskcreate's
   - added SIM7080-GM to PCB
   - added NANO sim holder to PCB
   - Upgraded MTCH6102 library due to bugs in writeRegister @ endTransmission()


  Sketch uses 2173468 bytes (69%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 69756 bytes (21%) of dynamic memory, leaving 257924 bytes for local variables. Maximum is 327680 bytes.



   13-02-2025 - ConfigAP, EEPROM save WiFi creds & others
   ====================================================================
   - added config-ap from v0.1 code
   - added eeprom save wifi credentials from config-ap
   - moved hursanet connection to its own void
   - updated ESP32 T1 TouchPin config
   - added DEBUG control for Serial.print lines


   #######
   ISSUES:
   #######
   - mpr121 random touch activations (only on pins with cables connected)
   - config-ap gets disabled by:
   - wifi autoreconnect doesnt stop but no longer blocks serial commands or buittons presses
   -


  Sketch uses 2247400 bytes (71%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 69972 bytes (21%) of dynamic memory, leaving 257708 bytes for local variables. Maximum is 327680 bytes.




   09/02/2025 - MVP Code formating & tidyup
   ====================================================================
   - rgbLedWrite() replaces depreciated neopixelWrite()
   - redo WifiMulti
   - added Hursanet VPN library
   - specified I2C pins 11 = SDA, 10 = SDA
   - added subscription mechanism


   #######
   ISSUES:
   #######
   - wifiMulti tries to connect 6 times then restarts..
   - i2c mpr121 seems to crash code.. could be chip is broken..
   -

  Sketch uses 2010448 bytes (63%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 68484 bytes (20%) of dynamic memory, leaving 259196 bytes for local variables. Maximum is 327680 bytes.



   28/01/2025 - ESP32-S3 & ESP32-SR (Confirmed working on ESP32-S3 hardware):
   ===================================================================
   - Compiled starting 9 Commands (python needed nltk & g2p-en)
   - Fixed issues with ESP32-SR "basic" - set to mono mode, 16bit, left
   - Confirmed working "basic" ESP32-SR with INMP441 & modified neopixel status indicator

    - FAILS to compile if MTCH activated in setup.. cannot write command as non existant atm
    - added WiFIMulti (multiple wifi ssids & swap to highest strength) - NEEDS FIXING
    - TO ADD: multi core wifi auto reconnect (seems to already be part of WiFiMulti)

  Sketch uses 1425020 bytes (45%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 57180 bytes (17%) of dynamic memory, leaving 270500 bytes for local variables. Maximum is 327680 bytes.




   25-27/01/2025 - ESP32-S3 & ESP32-SR (Confirmed working on ESP32-S3 hardware):
   ===================================================================
   - Updated neopixel control for ESP32-S3 with built-in controller:
   - neopixelWrite(RGB_BUILTIN, 150,150,150);
   - updated MQTT concept: not needed on vpn, can be anywhere on internet..
   - added Husranet as VPN (max 5 for free account..)





   24/01/2025 - ESP32-S3 & ESP32-SR (Test Compilations):
   ===================================================================
   - Updated all to ESP32-S3 due to:
   - ESP32-SR only compatible with ESP32-S3 on core V3.11
   - Updated ESP32 Arduino core to v3.11
   - LCARS File Browser not compiling on ESP core 3.1.1 - FIXED, SEE https://github.com/holgerlembke/ESPFMfGK/issues/8

   ~~~~~~~~~~~~~~~~~~ Working/Compiles ok ~~~~~~~~~~~~~~~~~~
   - Neopixel
   - I2S Mic
   - I2S Speech Recognition
   - Deep Sleep
   - Touch Wake
   - BadgeID
   - LCARS File Manager
   - MPR121
   - MTCH6102
   - SD Card
   - record to wav on SD Card
   - MAX17045

   Compile Settings:
    - USB CDC on
    - HUGE APP (no ota)

  Sketch uses 1081660 bytes (34%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 46324 bytes (14%) of dynamic memory, leaving 281356 bytes for local variables. Maximum is 327680 bytes.


   ~~~~~~~~~~~~~~~~~~ Needs Testing ~~~~~~~~~~~~~~~~~~
   - I2S record to SD
   - I2S stream to IP
   - send .wav to IP
   - download .wav from IP
   - MQTT
   - ESP32 Stream HTTP Client
   - ESP-SR add Command/Change Command
   -
   -

  =============================================================================================
  ----------------------------------ESP32-WROOM-32 Version-------------------------------------
  -----------------------------------------OUTDATED--------------------------------------------
  =============================================================================================

   22/01/2025:
   =============================
   - Removed RFM95
   - Added mSD Card
   - Split project into multiple .ino files
   - Renamed .ino files to correct alphabetical order
   - removed "download as zip" option in ESPFMfGK.h

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~
   -found HackCode/Libraies for:
     - LCARS Wifi File Browser ESPFMfGK                             - Compiles ok
     - ESP32 I2S Record to .wav on SD Card                          - compiles ok
     - Preferences NVS Storage                                      - Built-in Library - Needs adding to store BadgeID in NVS

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~
   - Added to Main Code:
      - LCARS Wifi File Browser ESPFMfGK
      - Deep Sleep & Touch Wake                                     - Needs adjusting for correct pins
      - SD Card                                                     - Compiles ok, works with Wifi File Manager, Need to check it stores .wav
      - BadgeID                                                     - Compiles ok
      - I2S Record .wav to SD Card                                  - needs testing with INMP441 but Compiles ok
      - NeoPixels                                                   - Need to change to GPIO16-ESP32-S3 or GPIO04-ESP32
      - External IP lookup                                          - needs testing
      - OTA Update                                                  - Activated by Serial Monitor: send "U"
      - MTCH6102                                                    - Added library but no init code
      - MAX17043 Lipo Monitor                                       - Added library but no init code
      - MPR121                                                      - Added library & init, needs button code

  Huge App 3Mb no OTA Partition/1Mb SPIFFS:
  Sketch uses 944365 bytes (30%) of program storage space. Maximum is 3145728 bytes.
  Global variables use 47044 bytes (14%) of dynamic memory, leaving 280636 bytes for local variables. Maximum is 327680 bytes.




   19/01/2025:
   =============================
   found HackCode/Libraies for:
     - record from I2S Mic & save .wav to Internal Storage        - Made for XIAO ESP32-S3 Sense, NEEDS MODIFYING
     - read line from file                                        - CURRENTLY ON SDFAT, NEEDS UPDATING TO LITTLEFS FOR INTERNAL/SD Card
     - ESP32 / ESP32-S3 Deep Sleep with Touch & EXT Wake          - Compiles ok
     - get external ip address                                    - Compiles ok
     - MTCH6102 Arduino Library                                   - Compiles ok

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Collated Libraries:    (Need adding to main code)
    - EEPROM Save WiFi
    - Chip ID (BadgeID)
    - WS2812b NeoPixel Controller
    - OTA Installer
    - On-Demand Config-AP
    - LittleFs Test
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Libraries still needed:
    - MQTT Receive data/file + save to storage
    - RFM95 (if using) - 22/01/2025: use SD Card
    -


   01/01/2025:
   =============================
   UPDATES FROM V0.1:

    New PCB
    New Casing
    Added MTCH6102 multi-gesture Trackpad controller as replacement for Main Capacitive Touch Panel.
    Upgraded large single electrode to custom Trackpad Array.
    Upgraded side TouchPads from Internal ESP32 TouchPins to MPR121 Pins.
    Upgraded ESP32-WROOM-32 to ESP32-S3-WROOM-1.
    Upgraded low level Class-D Audio Amplifier to I2S MAX98357 Audio Amplifier.
    Added RFM95 Lora-WiFi Adaptor for off-grid communication via Wireless Config-AP.
    Added WS2812b Notification/Status RGB LED
    Upgraded power converter output from 600ma to 1200ma
    Reduced battery size from 2000mAh to 1200mAh to reduce size
    =============================
*/


//EOF
