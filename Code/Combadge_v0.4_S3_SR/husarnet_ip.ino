/*  husarnet_ip.ino

    Persists the Husarnet IPv6 address across reboots and reconnects.

    After a successful Husarnet join, the assigned IPv6 address is written
    to NVS. On the next boot the stored address is available immediately,
    before Husarnet has finished rejoining, so MQTT and other services can
    use a stable address right away.

    The Husarnet ESP32 library always assigns the same IPv6 to a given join
    code + hostname pair, so the stored value stays valid indefinitely unless
    the badge is reprovisioned with a new join code, in which case the new
    address overwrites the old one automatically.

    NVS namespace : "hnet_ip"
    NVS key       : "ipv6"   (string, max 39 chars - full IPv6 text form)

    Functions:
      husarnetIPSetup()           - call once in setup(); loads cached IP into
                                    husarnetIPv6 global, or leaves it empty.
      saveHusarnetIP(ipString)    - write a new address to NVS; called internally
                                    by husarnet_task() after a successful join.
      getHusarnetIP()             - returns the cached IPv6 as a String.
      clearHusarnetIP()           - erases the stored address (use at reprovision).
*/

#define HNET_IP_NVS_NAMESPACE  "hnet_ip"
#define HNET_IP_NVS_KEY        "ipv6"
#define HNET_IP_MAX_LEN        40     // max text length of an IPv6 address + null

// ==================== Global: last known Husarnet IPv6 ====================
// Read by mqtt_presence.ino, keepwifi.ino, etc.
String husarnetIPv6 = "";   // empty until loaded or assigned

// ==================== Load from NVS at boot ====================

// Call once in setup(), after husarnetKeySetup().
// Populates husarnetIPv6 with the address from the previous session (if any).
void husarnetIPSetup() {
  Preferences prefs;
  prefs.begin(HNET_IP_NVS_NAMESPACE, true);   // read-only
  String stored = prefs.getString(HNET_IP_NVS_KEY, "");
  prefs.end();

  if (stored.length() > 0) {
    husarnetIPv6 = stored;
    debug("HusarnetIP: loaded cached IPv6 from NVS: ");
    debugln(husarnetIPv6);
  } else {
    debugln("HusarnetIP: no cached IPv6 in NVS (first boot or cleared).");
  }
}

// ==================== Save to NVS after successful join ====================

// Writes ipString to NVS.
// Called by husarnet_task() immediately after husarnet.isJoined() is true.
// Safe to call on every reconnect - NVS wear is negligible for infrequent writes.
bool saveHusarnetIP(const String& ipString) {
  if (ipString.length() == 0 || ipString.length() >= HNET_IP_MAX_LEN) {
    debugln("HusarnetIP: invalid IP string - not saving.");
    return false;
  }

  Preferences prefs;
  prefs.begin(HNET_IP_NVS_NAMESPACE, false);  // read-write
  size_t written = prefs.putString(HNET_IP_NVS_KEY, ipString);
  prefs.end();

  if (written == 0) {
    debugln("HusarnetIP: NVS write failed.");
    return false;
  }

  husarnetIPv6 = ipString;   // keep global in sync
  debug("HusarnetIP: IPv6 saved to NVS: ");
  debugln(husarnetIPv6);
  return true;
}

// ==================== Accessor ====================

String getHusarnetIP() {
  return husarnetIPv6;
}

// ==================== Clear (call on reprovision) ====================

void clearHusarnetIP() {
  Preferences prefs;
  prefs.begin(HNET_IP_NVS_NAMESPACE, false);
  prefs.remove(HNET_IP_NVS_KEY);
  prefs.end();
  husarnetIPv6 = "";
  debugln("HusarnetIP: cached IPv6 cleared from NVS.");
}

//EOF
