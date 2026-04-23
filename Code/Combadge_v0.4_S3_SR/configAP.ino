//On-Demand Config AP:
void smartConfig() {
//  LED_1_WAIT;
  ESP_SR.setMode(SR_MODE_OFF);
  if (!EEPROM.begin(EEPROM_SIZE)) { //Init EEPROM
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ failed to init EEPROM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    delay(1000);
  }
  else
  {
    esid = readStringFromFlash(0);   // Read esid stored at address 0
    Serial.print("esid = ");
    debugln(esid);
    epass = readStringFromFlash(40); // Read Password stored at address 40
    Serial.print("epass = ");
    debugln(epass);
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    delay(500);
  }

  WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  delay(3500); // Wait for a while till ESP connects to WiFi

  if (WiFi.status() != WL_CONNECTED) // if WiFi is not connected
  {
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
    delay(1000);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    debugln("Launching Captive Portal...");
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");


    //WiFiManager wifiManager;
    //wifiManager.resetSettings();
    //wifiManager.setTimeout(120);
    wifiManager.setConfigPortalBlocking(false);
    
    String apName = "VoxAir_" + BadgeID;
    wifiManager.startConfigPortal(apName.c_str(), "ConnectMe");

    while (WiFi.status() != WL_CONNECTED) {
      wifiManager.process();
      delay(100);
    }

    debugln("Portal config received.");
    //LED_2_OK;
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    debugln("WiFi Connected.");
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    //LED_1_OK;
    Serial.print("IP Address: ");
    debugln(WiFi.localIP());
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    // Read the connected WiFi esid and password
    esid = WiFi.SSID();
    epass = WiFi.psk();
    Serial.print("SSID:");
    debugln(esid);
    Serial.print("PSWD:");
    debugln(epass);
    addWifiNetwork(esid, epass);
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    debugln("Storing esid & epass in Flash..");
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    writeStringToFlash(esid.c_str(), 0);   // storing esid at address 0
    writeStringToFlash(epass.c_str(), 40); // storing epass at address 40
    debugln("Credentials Saved!");
   // LED_1_OK;
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    ESP.restart();
  }
  else
  {
    debugln("WiFi Connected");
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    ESP_SR.setMode(SR_MODE_OFF);
    wifiReady = true;
  }
}
//EOF End-Of-File
