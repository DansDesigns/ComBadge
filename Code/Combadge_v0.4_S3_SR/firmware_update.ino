/*  firmware_update.ino
 *
 *  firmwareUpdate()        - downloads and flashes new firmware .bin from backend
 *  FirmwareVersionCheck()  - returns 1 if newer firmware available, 0 if current
 *  keyFileVersionCheck()   - returns 1 if a new /husarnet.key is available
 *  checkAndUpdate()        - full update cycle: check version, update firmware,
 *                            check for new key file, download if available
 *
 *  Backend URL scheme (all on your GitHub or server):
 *    version.txt       - plain text firmware version number e.g. "0.3"
 *    fw.bin            - compiled firmware binary
 *    keyversion.txt    - plain text key file version/date e.g. "2025-06"
 *    husarnet.key      - AES-128 encrypted Husarnet join code (binary)
 *
 *  The key file version is stored in NVS so the badge knows if it already
 *  has the latest one. On monthly renewal the backend updates keyversion.txt
 *  and husarnet.key together.
 */

#define URL_key_Version  "https://raw.githubusercontent.com/DansDesigns/ComBadge/main/firmware/keyversion.txt"
#define URL_key_File     "https://raw.githubusercontent.com/DansDesigns/ComBadge/main/firmware/husarnet.key"

#define NVS_KEY_VER_NS   "hnet"
#define NVS_KEY_VER_KEY  "keyver"

// ==================== Firmware update ====================

void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n",
                    httpUpdate.getLastError(),
                    httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      debugln("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      debugln("HTTP_UPDATE_OK");
      break;
  }
}

int FirmwareVersionCheck(void) {
  String payload  = "";
  int    httpCode = -1;

  String FirmwareURL = String(URL_fw_Version) + "?" + String(rand());
  debugln(FirmwareURL);

  WiFiClientSecure* client = new WiFiClientSecure;
  if (client) {
    client->setCACert(rootCACertificate);
    HTTPClient https;

    if (https.begin(*client, FirmwareURL)) {
      debugln("[HTTPS] Checking firmware version...");
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) {
        payload = https.getString();
      } else {
        debug("Version check HTTP error: ");
        debugln(httpCode);
      }
      https.end();
    } else {
      debugln("Version check: https.begin() failed.");
    }
    delete client;
  }

  if (httpCode == HTTP_CODE_OK) {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      debug("Already on latest firmware: ");
      debugln(FirmwareVer);
      return 0;
    } else {
      debug("New firmware available: ");
      debugln(payload);
      return 1;
    }
  }
  return 0;
}

// ==================== Key file update ====================

// Fetches the current key version string from the backend.
// Returns empty string on failure.
String fetchRemoteKeyVersion() {
  String version  = "";
  int    httpCode = -1;

  String url = String(URL_key_Version) + "?" + String(rand());

  WiFiClientSecure* client = new WiFiClientSecure;
  if (client) {
    client->setCACert(rootCACertificate);
    HTTPClient https;
    if (https.begin(*client, url)) {
      httpCode = https.GET();
      if (httpCode == HTTP_CODE_OK) {
        version = https.getString();
        version.trim();
      }
      https.end();
    }
    delete client;
  }
  return version;
}

// Returns 1 if a new key file is available, 0 if up to date, -1 on error.
int keyFileVersionCheck() {
  String remoteVer = fetchRemoteKeyVersion();
  if (remoteVer.isEmpty()) {
    debugln("KeyUpdate: could not fetch remote key version.");
    return -1;
  }

  // Read locally stored key version from NVS
  Preferences prefs;
  prefs.begin(NVS_KEY_VER_NS, true);
  String localVer = prefs.getString(NVS_KEY_VER_KEY, "");
  prefs.end();

  debug("KeyUpdate: local=");
  debug(localVer);
  debug(" remote=");
  debugln(remoteVer);

  if (remoteVer.equals(localVer)) {
    debugln("KeyUpdate: key file is current.");
    return 0;
  }

  debugln("KeyUpdate: new key file available.");
  return 1;
}

// Downloads the new key file from backend, saves to SD, updates NVS version.
bool downloadKeyFile() {
  String remoteVer = fetchRemoteKeyVersion();
  if (remoteVer.isEmpty()) return false;

  debugln("KeyUpdate: downloading new /husarnet.key...");

  // updateKeyFile() is defined in husarnet_key.ino
  if (!updateKeyFile(String(URL_key_File))) {
    debugln("KeyUpdate: download failed.");
    return false;
  }

  // Save new version string to NVS so we don't re-download next boot
  Preferences prefs;
  prefs.begin(NVS_KEY_VER_NS, false);
  prefs.putString(NVS_KEY_VER_KEY, remoteVer);
  prefs.end();

  debug("KeyUpdate: key file updated to version: ");
  debugln(remoteVer);

  // Reload the join code from the new file immediately
  if (loadJoinCode(husarnetJoinCode)) {
    debugln("KeyUpdate: join code reloaded from new key file.");
  } else {
    debugln("KeyUpdate: WARNING - new key file downloaded but failed to decrypt.");
    debugln("KeyUpdate: badge may need re-provisioning.");
  }

  return true;
}

// ==================== Full update cycle ====================
// Call this on boot (after WiFi connects) or on a schedule.
// Checks firmware version, updates if needed, then checks key file.

void checkAndUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    debugln("Update: WiFi not connected, skipping.");
    return;
  }

  debugln("Update: checking for updates...");

  // Check firmware
  if (FirmwareVersionCheck() == 1) {
    debugln("Update: new firmware found, updating...");
    firmwareUpdate();
    // If update succeeds the device reboots automatically.
    // Code below only runs if update failed.
    debugln("Update: firmware update failed, continuing.");
  }

  // Check key file (subscription renewal)
  int keyStatus = keyFileVersionCheck();
  if (keyStatus == 1) {
    downloadKeyFile();
  } else if (keyStatus == -1) {
    debugln("Update: could not check key version - no internet or server down.");
  }
}


//EOF
