/*
 * ip.ino
 *
 * external_ip()    — fetches WAN IP from ipify, stores in Preferences if changed.
 *                    Still useful as a diagnostic / fallback address.
 *
 * getHusarnetIP()  — returns the badge's Husarnet IPv6 address as a String.
 *                    This is what will be published over MQTT so contacts can
 *                    reach this badge from anywhere, regardless of NAT or
 *                    which connection type is active (WiFi / LTE / HaLow).
 */

void external_ip() {

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;
  String payload = "";

  http.begin(client, textURL);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    debug("WAN IP HTTP response: ");
    debugln(httpResponseCode);
    payload = http.getString();
    payload.trim();
    debug("WAN IP: ");
    debugln(payload);

    IPAddress wanIP;
    if (wanIP.fromString(payload)) {
      uint32_t wanIP32bits = (uint32_t)wanIP;

      preferences.begin("IPStorage", false);
      uint32_t oldIP32bits = preferences.getULong("oldIP", 0xFFFFFFFF);

      if (oldIP32bits != wanIP32bits) {
        debugln("WAN IP changed, updating stored value.");
        preferences.putULong("oldIP", wanIP32bits);
      } else {
        debugln("WAN IP unchanged.");
      }
      preferences.end();
    }
  } else {
    debug("WAN IP fetch failed, code: ");
    debugln(httpResponseCode);
  }

  http.end();
}

// Returns Husarnet IPv6 address as String, or empty string if not yet joined.
// This is the address to publish via MQTT — stable across WiFi, LTE, HaLow.

//String getHusarnetIP() {
//  if (!husarnet_connected) return "";
////  LED_1_OK;
//  return String(husarnet.getIpAddress().c_str());
//}


//EOF
