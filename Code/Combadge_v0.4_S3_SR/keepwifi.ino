/*
  WiFi Status Codes:
  WL_IDLE_STATUS    = 0
  WL_NO_SSID_AVAIL  = 1
  WL_SCAN_COMPLETED = 2
  WL_CONNECTED      = 3
  WL_CONNECT_FAILED = 4
  WL_CONNECTION_LOST= 5
  WL_DISCONNECTED   = 6
  WL_NO_SHIELD      = 255
*/

// Returns true if connected, false if connection failed and SmartConfig was launched.
bool keepWifiAlive()
{
  WiFi.mode(WIFI_STA);
  
  //  WiFi.begin(esid.c_str(), epass.c_str());
  
  if (!connectBestWifi()) {
    smartConfig(); 
  }
  
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // needed on ESP32-S3, see espressif/arduino-esp32#6767
  debugln("WiFi mode: WIFI_STA");
  unsigned long startAttemptTime = millis();
  //
  //  //if connected do nothing and quit
  //  if (WiFi.status() == WL_CONNECTED)
  //  {
  //    WiFi.setAutoReconnect(true);
  //    //~~~~~~~~~LED_2_OK;
  //    debug("WiFi connected: ");
  //    debugln(WiFi.localIP());
  //    external_ip();
  //    delay(500);
  //
  //    //    husarnet.join(HOSTNAME, JOIN_CODE);
  //    //    while (!husarnet.isJoined())
  //    //    {
  //    //      debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  //    //      debugln("Waiting for Husarnet network...");
  //    //      activate_husarnet();
  //  }


  //initiate connection
  debugln("WiFi Connecting");

  //Indicate to the user that we are not currently connected but are trying to connect.
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    //LED_1_WAIT;
    debug(".");
    delay(500);
    continue;
  }

  //Indicate to the user the outcome of our attempt to connect.
  if (WiFi.status() == WL_CONNECTED)
  {
    debug("WIFI Connected: ");
    debugln(WiFi.localIP());
    //LED_1_OK;
    external_ip();
    delay(500);

    //    husarnet.join(HOSTNAME, JOIN_CODE);
    //    while (!husarnet.isJoined())
    //    {
    //      debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    //      debugln("Waiting for Husarnet network...");
    //      activate_husarnet();
    return true;
  }

  else
  {
    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WIFI Failed to Connect: timeout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    LED_1_ERROR;

    debugln("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Config-AP Starting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    smartConfig();
    return false;
  }
}

// ==================== EEPROM Helpers ====================

void writeStringToFlash(const char* toStore, int startAddr)
{
  int i = 0;
  for (; i < strlen(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}

String readStringFromFlash(int startAddr)
{
  char in[128];
  for (int i = 0; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}


//EOF
