
//BadgeID:
uint32_t chipId = 0;      //variable to store the BadgeID

void badgeid() {
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.print("BadgeID: ");
  Serial.println(chipId);
  Serial.print("Active Firmware Version:");
  Serial.println(FirmwareVer);
  Serial.print("CONNECTED TO: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  Serial.print("SIGNAL: ");
  Serial.print(WiFi.RSSI());
}

//EOF End-Of-File
