/*  mqtt_presence.ino  (UDP rewrite — no MQTT broker required)
 *
 *  Peer-to-peer signalling over Husarnet IPv6 UDP.
 *  Replaces the MQTT broker with direct badge-to-badge UDP packets.
 *  No central server, no broker, no library dependencies beyond WiFiUdp.h.
 *
 *  How it works:
 *  ─────────────
 *  PRESENCE
 *    On boot (once Husarnet joins) each badge sends a PRESENCE packet to
 *    every known contact's Husarnet IPv6 address.  This tells them our
 *    current IP.  We also listen for their PRESENCE packets and call
 *    updateContactIP() when one arrives — exactly as the MQTT version did.
 *
 *    There is no broker to store retained messages, so we re-broadcast
 *    presence periodically (PRESENCE_INTERVAL_MS) so any badge that comes
 *    online later will catch it within one interval.
 *
 *  CALL SIGNALLING  (ring / accept / reject / end)
 *    mqttPublishCallSignal(targetBadgeID, action) looks up the contact's
 *    stored Husarnet IP and sends a UDP packet directly to them.
 *    Incoming packets are parsed in udpLoop() and dispatched to
 *    handleIncomingCallSignal() — same call_manager.ino handler as before.
 *
 *  Packet format  (plain text, kept small for UDP)
 *    PRESENCE|<fromBadgeID>|<husarnetIPv6>|<fwVersion>
 *    CALL|<fromBadgeID>|<fromIP>|<action>
 *
 *  Port:  COMBADGE_UDP_PORT (default 7240) — must be the same on all badges.
 *
 *  Public API (identical to the old mqtt_presence.ino so nothing else changes):
 *    void mqttSetup()
 *    void mqttLoop()
 *    void mqttPublishCallSignal(const String& targetBadgeID, const String& action)
 *    bool mqttReady   — true once Husarnet is up and UDP socket is open
 */

#include <WiFiUdp.h>

// ==================== Configuration ====================

#define COMBADGE_UDP_PORT       7240    // UDP port all badges listen on

// How often to re-broadcast our presence to all contacts (ms)
#define PRESENCE_INTERVAL_MS    30000UL

// How long to wait after Husarnet joins before first presence broadcast (ms)
// Gives Husarnet a moment to fully settle before we start sending.
#define PRESENCE_BOOT_DELAY_MS  3000UL

// Max UDP packet size — our packets are well under this
#define UDP_RX_BUFFER           256

// ==================== State ====================

WiFiUDP        udp;
bool           mqttReady            = false;   // true = UDP socket open & Husarnet up
bool           udpSocketOpen        = false;
unsigned long  lastPresenceBroadcast = 0;
unsigned long  husarnetReadyTime     = 0;      // millis() when Husarnet first joined


// ==================== Internal helpers ====================

// Returns our current Husarnet IPv6 string, or LAN IP as fallback.
static String myIP() {
  if (husarnet_connected) return String(husarnet.getIpAddress().c_str());
  return WiFi.localIP().toString();
}

// Open (or reopen) the UDP listening socket.
static bool openUdpSocket() {
  if (udpSocketOpen) return true;
  if (udp.begin(COMBADGE_UDP_PORT)) {
    udpSocketOpen = true;
    debug("UDP: listening on port ");
    debugln(COMBADGE_UDP_PORT);
    return true;
  }
  debugln("UDP: failed to open socket");
  return false;
}

// ==================== Presence broadcast ====================

// Build a PRESENCE packet string.
static String buildPresencePacket() {
  // PRESENCE|<badgeID>|<ip>|<fw>
  return String("PRESENCE|") + BadgeID + "|" + myIP() + "|" + FirmwareVer;
}

// Send our presence to a single IPv6 address.
static void sendPresenceTo(const String& ipv6) {
  if (ipv6.isEmpty()) return;
  String pkt = buildPresencePacket();
  udp.beginPacket(ipv6.c_str(), COMBADGE_UDP_PORT);
  udp.write((const uint8_t*)pkt.c_str(), pkt.length());
  udp.endPacket();
  debug("UDP presence → ");
  debugln(ipv6);
}

// Broadcast our presence to all known contacts.
static void broadcastPresence() {
  if (!mqttReady) return;
  for (int i = 0; i < contactCount; i++) {
    String ip = String(contacts[i].ip);   // stored Husarnet IP from ca_contacts.ino
    sendPresenceTo(ip);
  }
  debugln("UDP: presence broadcast done.");
//  LED_1_OK;
  // add led colour code here... sent update saying "my ip is...."
}


// ==================== Packet parser ====================

// Parse and dispatch a received UDP packet string.
static void handleUdpPacket(const String& pkt, const String& senderIP) {

  // ---- PRESENCE packet ----
  // Format: PRESENCE|<fromBadgeID>|<ip>|<fw>
  if (pkt.startsWith("PRESENCE|")) {
    int f1 = pkt.indexOf('|', 9);           // end of "PRESENCE"
    int f2 = (f1 > 0) ? pkt.indexOf('|', f1 + 1) : -1;

    if (f1 < 0 || f2 < 0) return;          // malformed

    String fromID = pkt.substring(9, f1);
    String fromIP = pkt.substring(f1 + 1, f2);
    // fw field is after f2 — not used here but available if needed

    if (fromID.equals(BadgeID)) return;     // our own reflection, ignore

    debug("UDP presence rx from ");
    debug(fromID);
    debug(" @ ");
    debugln(fromIP);

    updateContactIP(fromID, fromIP);        // ca_contacts.ino

    // Reply with our own presence so they have our IP too
    sendPresenceTo(fromIP);
    return;
  }

  // ---- CALL signal packet ----
  // Format: CALL|<fromBadgeID>|<fromIP>|<action>
  if (pkt.startsWith("CALL|")) {
    int f1 = pkt.indexOf('|', 5);
    int f2 = (f1 > 0) ? pkt.indexOf('|', f1 + 1) : -1;
    int f3 = (f2 > 0) ? pkt.indexOf('|', f2 + 1) : -1;

    if (f1 < 0 || f2 < 0 || f3 < 0) return;  // malformed

    String fromID  = pkt.substring(5, f1);
    String fromIP  = pkt.substring(f1 + 1, f2);
    String action  = pkt.substring(f3 + 1);

    debug("UDP call rx from ");
    debug(fromID);
    debug(" action=");
    debugln(action);

    // Build a JSON payload in the same shape handleIncomingCallSignal() expects
    // so call_manager.ino needs zero changes.
    String jsonPayload = "{";
    jsonPayload += "\"from\":\"" + fromID + "\",";
    jsonPayload += "\"ip\":\""   + fromIP + "\",";
    jsonPayload += "\"action\":\"" + action + "\"";
    jsonPayload += "}";

    handleIncomingCallSignal(jsonPayload);  // call_manager.ino
    return;
  }

  // Unknown packet type — ignore
  debug("UDP: unknown packet: ");
  debugln(pkt);
}


// ==================== Public API ====================

// Called from zsetup.ino — replaces mqttSetup().
// Just opens the UDP socket; Husarnet may not be up yet so mqttReady
// is set later in mqttLoop() once husarnet_connected goes true.
void mqttSetup() {
  openUdpSocket();
  debugln("UDP signalling: configured.");
}

// Called from zzloop.ino every loop iteration.
void mqttLoop() {

  // ---- Gate on Husarnet being up ----
  if (!husarnet_connected) return;

  // First time Husarnet becomes ready
  if (!mqttReady) {
    if (!udpSocketOpen) openUdpSocket();
    if (udpSocketOpen) {
      mqttReady = true;
      husarnetReadyTime = millis();
      debugln("UDP: mqttReady — Husarnet up.");
    }
    return;   // wait until next loop so husarnetReadyTime is set
  }

  // ---- Boot presence delay ----
  // Don't spam contacts the instant Husarnet joins; give it a few seconds.
  unsigned long now = millis();
  if (now - husarnetReadyTime < PRESENCE_BOOT_DELAY_MS) return;

  // ---- Periodic presence broadcast ----
  if (now - lastPresenceBroadcast > PRESENCE_INTERVAL_MS || lastPresenceBroadcast == 0) {
    lastPresenceBroadcast = now;
    broadcastPresence();
  }

  // ---- Receive incoming UDP packets ----
  int packetSize = udp.parsePacket();
  while (packetSize > 0) {
    char buf[UDP_RX_BUFFER];
    int len = udp.read(buf, sizeof(buf) - 1);
    if (len > 0) {
      buf[len] = '\0';
      String senderIP = udp.remoteIP().toString();
      handleUdpPacket(String(buf), senderIP);
    }
    packetSize = udp.parsePacket();   // check for more queued packets
  }
}

// Called from call_manager.ino — replaces mqtt.publish() for call signals.
// Looks up the target's stored Husarnet IP and sends a UDP CALL packet.
void mqttPublishCallSignal(const String& targetBadgeID, const String& action) {
  if (!mqttReady) {
    debugln("UDP: not ready, cannot send call signal.");
    return;
  }

  // Find the contact's IP
  String targetIP = "";
  for (int i = 0; i < contactCount; i++) {
    if (String(contacts[i].badgeID).equals(targetBadgeID)) {
      targetIP = String(contacts[i].ip);
      break;
    }
  }

  if (targetIP.isEmpty()) {
    debug("UDP: no IP for contact ");
    debugln(targetBadgeID);
    return;
  }

  // CALL|<fromBadgeID>|<fromIP>|<action>
  String pkt = String("CALL|") + BadgeID + "|" + myIP() + "|" + action;

  udp.beginPacket(targetIP.c_str(), COMBADGE_UDP_PORT);
  udp.write((const uint8_t*)pkt.c_str(), pkt.length());
  udp.endPacket();

  debug("UDP call → ");
  debug(targetIP);
  debug(" action=");
  debugln(action);
}

// EOF
