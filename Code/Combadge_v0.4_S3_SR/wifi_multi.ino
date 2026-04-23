/*  wifi_multi.ino

    Stores up to WIFI_MAX_NETWORKS SSIDs + passwords in NVS and connects
    to whichever saved network has the strongest signal (highest RSSI).

    Replaces the single esid/epass EEPROM approach in keepwifi.ino.
    keepwifi.ino's keepWifiAlive() should call connectBestWifi() instead
    of WiFi.begin(esid, epass).

    NVS namespace : "wifi_multi"
    Keys          : "count"          (uint8  - number of stored networks)
                    "ssid_0".."ssid_N"  (string)
                    "pass_0".."pass_N"  (string)

    Functions:
      wifiMultiSetup()                       - load network count from NVS, call in setup()
      addWifiNetwork(ssid, password)         - save a new network (or update existing)
      removeWifiNetwork(ssid)                - delete a network by SSID
      listWifiNetworks()                     - print all stored SSIDs to Serial
      connectBestWifi()                      - scan & connect to strongest known network
                                               returns true if connected
      getConnectedSSID()                     - returns currently connected SSID string
      wifiNetworkCount()                     - returns number of stored networks

    Usage in keepwifi.ino / setup():
      wifiMultiSetup();
      ...
      if (!connectBestWifi()) { smartConfig(); }
*/

//#include <WiFi.h>
//#include <Preferences.h>

#define WIFI_MULTI_NVS_NS   "wifi_multi"
#define WIFI_MAX_NETWORKS   10          // max stored networks
#define WIFI_SCAN_TIMEOUT   6000        // ms to wait for scan
#define WIFI_CONN_TIMEOUT   10000       // ms to wait for connection

static uint8_t wifiNetCount = 0;       // loaded from NVS at setup

// ==================== Setup ====================

void wifiMultiSetup() {
  Preferences prefs;
  prefs.begin(WIFI_MULTI_NVS_NS, true);
  wifiNetCount = prefs.getUChar("count", 0);
  prefs.end();
  debug("WiFiMulti: ");
  debug(wifiNetCount);
  debugln(" network(s) stored in NVS.");
}

// ==================== Add / Update ====================

// Adds a new network or updates the password if SSID already exists.
// Returns false if storage is full.
bool addWifiNetwork(const String& ssid, const String& password) {
  if (ssid.length() == 0) {
    debugln("WiFiMulti: SSID cannot be empty.");
    return false;
  }

  Preferences prefs;
  prefs.begin(WIFI_MULTI_NVS_NS, false);

  // Check if SSID already stored - update password if so
  for (uint8_t i = 0; i < wifiNetCount; i++) {
    String key = "ssid_" + String(i);
    if (prefs.getString(key.c_str(), "") == ssid) {
      String pkey = "pass_" + String(i);
      prefs.putString(pkey.c_str(), password);
      prefs.end();
      debug("WiFiMulti: updated password for: ");
      debugln(ssid);
      return true;
    }
  }

  // New entry
  if (wifiNetCount >= WIFI_MAX_NETWORKS) {
    prefs.end();
    debugln("WiFiMulti: storage full - remove a network first.");
    return false;
  }

  String ssidKey = "ssid_" + String(wifiNetCount);
  String passKey = "pass_" + String(wifiNetCount);
  prefs.putString(ssidKey.c_str(), ssid);
  prefs.putString(passKey.c_str(), password);
  wifiNetCount++;
  prefs.putUChar("count", wifiNetCount);
  prefs.end();

  debug("WiFiMulti: added network: ");
  debugln(ssid);
  return true;
}

// ==================== Remove ====================

// Removes a network by SSID. Compacts the list so there are no gaps.
bool removeWifiNetwork(const String& ssid) {
  Preferences prefs;
  prefs.begin(WIFI_MULTI_NVS_NS, false);

  int foundIdx = -1;
  for (uint8_t i = 0; i < wifiNetCount; i++) {
    String key = "ssid_" + String(i);
    if (prefs.getString(key.c_str(), "") == ssid) {
      foundIdx = i;
      break;
    }
  }

  if (foundIdx == -1) {
    prefs.end();
    debug("WiFiMulti: SSID not found: ");
    debugln(ssid);
    return false;
  }

  // Shift remaining entries down to fill the gap
  for (uint8_t i = foundIdx; i < wifiNetCount - 1; i++) {
    String nextSsid = prefs.getString(("ssid_" + String(i + 1)).c_str(), "");
    String nextPass = prefs.getString(("pass_" + String(i + 1)).c_str(), "");
    prefs.putString(("ssid_" + String(i)).c_str(), nextSsid);
    prefs.putString(("pass_" + String(i)).c_str(), nextPass);
  }

  // Clear the last slot
  prefs.remove(("ssid_" + String(wifiNetCount - 1)).c_str());
  prefs.remove(("pass_" + String(wifiNetCount - 1)).c_str());
  wifiNetCount--;
  prefs.putUChar("count", wifiNetCount);
  prefs.end();

  debug("WiFiMulti: removed network: ");
  debugln(ssid);
  return true;
}

// ==================== List ====================

void listWifiNetworks() {
  if (wifiNetCount == 0) {
    debugln("WiFiMulti: no networks stored.");
    return;
  }
  Preferences prefs;
  prefs.begin(WIFI_MULTI_NVS_NS, true);
  debugln("WiFiMulti: stored networks:");
  for (uint8_t i = 0; i < wifiNetCount; i++) {
    String s = prefs.getString(("ssid_" + String(i)).c_str(), "");
    debug("  ["); debug(i); debug("] "); debugln(s);
  }
  prefs.end();
}

// ==================== Connect to Best ====================

// Scans for visible APs, cross-references stored networks, picks
// the one with the highest RSSI, and connects.
// Returns true if connected within WIFI_CONN_TIMEOUT ms.
bool connectBestWifi() {
  if (wifiNetCount == 0) {
    debugln("WiFiMulti: no networks stored - cannot connect.");
    return false;
  }

  debugln("WiFiMulti: scanning for networks...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  int found = WiFi.scanNetworks(false, false, false, WIFI_SCAN_TIMEOUT);

  if (found <= 0) {
    debugln("WiFiMulti: scan found no networks.");
    WiFi.scanDelete();
    return false;
  }

  debug("WiFiMulti: scan found ");
  debug(found);
  debugln(" visible network(s).");

  // Load stored credentials
  Preferences prefs;
  prefs.begin(WIFI_MULTI_NVS_NS, true);

  String bestSSID     = "";
  String bestPassword = "";
  int32_t bestRSSI    = -9999;

  for (int s = 0; s < found; s++) {
    String scannedSSID = WiFi.SSID(s);
    int32_t rssi       = WiFi.RSSI(s);

    // Check if this scanned SSID is in our stored list
    for (uint8_t n = 0; n < wifiNetCount; n++) {
      String storedSSID = prefs.getString(("ssid_" + String(n)).c_str(), "");
      if (storedSSID == scannedSSID) {
        debug("WiFiMulti: found known network: ");
        debug(scannedSSID);
        debug("  RSSI: ");
        debugln(rssi);
        if (rssi > bestRSSI) {
          bestRSSI     = rssi;
          bestSSID     = storedSSID;
          bestPassword = prefs.getString(("pass_" + String(n)).c_str(), "");
        }
        break;  // no need to check remaining stored entries for this scan result
      }
    }
  }

  prefs.end();
  WiFi.scanDelete();

  if (bestSSID.isEmpty()) {
    debugln("WiFiMulti: no stored networks are in range.");
    return false;
  }

  debug("WiFiMulti: connecting to best network: ");
  debug(bestSSID);
  debug(" (RSSI ");
  debug(bestRSSI);
  debugln(")");

  WiFi.begin(bestSSID.c_str(), bestPassword.c_str());
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // S3 workaround

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONN_TIMEOUT) {
    delay(300);
    debug(".");
  }
  debugln("");

  if (WiFi.status() == WL_CONNECTED) {
    debug("WiFiMulti: connected to ");
    debug(bestSSID);
    debug("  IP: ");
    debugln(WiFi.localIP());
    return true;
  }

  debug("WiFiMulti: failed to connect to ");
  debugln(bestSSID);
  return false;
}

// ==================== Accessors ====================

String getConnectedSSID() {
  return WiFi.SSID();
}

uint8_t wifiNetworkCount() {
  return wifiNetCount;
}

//EOF
