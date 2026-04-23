void setup()
{
  LED_1_BOOT;
  LED_2_BOOT;

  Serial.begin(115200);
  Wire.begin(10, 11);   // SDA=10, SCL=11
  sdSPI.begin(sck, miso, mosi, cs);

  // BadgeID now runs inline, previously launched as a FreeRTOS task
  // which caused a race condition (BadgeID could be empty when first used).
  initBadgeID();

  print_wakeup_reason();
  pinMode(WiFi_rst, INPUT);
  pinMode(MTCH6102_Interupt_Pin, INPUT);

  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);  // wake on LOW from MTCH6102

  // ---- I2S must be configured before ESP-SR ----
  i2s.setPins(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN);
  i2s.setTimeout(10000);


  // ---- I2S_NUM_0: dedicated RX for ESP-SR mic input ----
  i2s.setPins(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN);
  i2s.setTimeout(10000);
  i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT);
  debugln("I2S setup done.");


  ESP_SR.onEvent(onSrEvent);
  // NOTE: ESP_SR.begin() is called later (after loadContacts) so that
  // contact names are included in the command table.  See buildSrCommands().
  debugln("ESP-SR event handler registered (begin deferred until contacts load).");


  // ---- I2S_NUM_1: dedicated TX channel for speaker output ----
  soundsSetup();
  debugln("Sounds setup done.");


  wifiMultiSetup();   // ← replaces loading esid/epass from EEPROM

  //  OLD METHOD:
  // ---- EEPROM: read saved WiFi credentials ----
  //  if (!EEPROM.begin(EEPROM_SIZE)) {
  //    debugln("ERROR: EEPROM init failed.");
  //    delay(200);
  //  } else {
  //    esid  = readStringFromFlash(0);   // SSID at address 0
  //    epass = readStringFromFlash(40);  // Password at address 40
  //    debug("SSID: ");  debugln(esid);
  //    debug("Pass: ");  debugln(epass);
  //  }

  debugln("Testing SD...");


  // ---- SD Card: retry init up to 3 times ----
  bool sdMounted = false;
  for (int i = 0; i < 3; i++) {
    if (SD.begin(cs, sdSPI, 4000000)) {
      sdMounted = true;
      break;
    }
    debugln("SD init failed, retrying...");
    delay(500);
  }
  if (!sdMounted) {
    debugln("WARNING: SD card not found or init failed. Continuing without SD.");
  } else {
    debugln("SD card mounted OK.");
  }


  // ---- Load SD sound overrides (must be after SD.begin()) ----
  // If /chirp.wav exists on the SD card it will be used for SOUND_CHIRP;
  // otherwise the built-in chirp.h data is used automatically.
  soundsLoadFromSD();

  // ---- Boot chirp ----
  playSoundAsync(SOUND_CHIRP);


  // ---- MTCH6102: set gesture-only mode (0x01) ----
  Wire.beginTransmission(ADDR);
  Wire.write(0x05);   // MODE register
  Wire.write(0x01);   // Gesture only
  byte busStatus = Wire.endTransmission();
  if (busStatus != 0x00) {
    debug("MTCH6102 I2C error: ");
    debugln(busStatus);
  } else {
    debugln("MTCH6102 setup done.");
  }

  //LED_1_WAIT;

  LED_1_OK;
  LED_2_OK;

  // ---- WiFi ----
  keepWifiAlive();


  // ---- Husarnet key ----
  // Load AES key from NVS, decrypt join code from SD card.
  // Must run after SD.begin() but before startHusarnet(), as it uses husarnetJoinCode.
  husarnetKeySetup();
  husarnetIPSetup();
  loadJoinCode(husarnetJoinCode);

  // ---- Husarnet VPN ----
  // Started after WiFi, runs in its own task on Core 0 with 16KB stack.
  // Connects asynchronously so setup() returns immediately.
  // husarnet_connected flag is set true when join completes.
  startHusarnet();


  // ---- Contacts ----
  // Load address book from SD (/contacts.json).
  // Must run after SD.begin() and before MQTT setup.
  loadContacts();
  printContacts();

  // ---- ESP-SR: build dynamic command table from contacts ----
  // Constructs [fixed cmds] + [one "Call <Name>" per contact], then
  // calls ESP_SR.begin() so the recogniser knows all valid utterances.
  buildSrCommands();
  ESP_SR.begin(i2s, sr_commands_dynamic, sr_commands_count, SR_CHANNELS_MONO, SR_MODE_OFF);
  debugln("ESP-SR setup done (dynamic commands including contacts).");

  // ---- MQTT Presence ----
  // Configures broker and callback. Actual TCP connection happens
  // on first mqttLoop() call, non-blocking from setup().
  mqttSetup();

  // ---- Call Manager ----
  callManagerSetup();

  wifiReady = true;
  LED_1_OFF;
  LED_2_OFF;
}
//EOF
