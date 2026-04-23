/*  call_manager.ino

    Call state machine — connects speech recognition, MQTT signalling,
    and audio streaming into a complete call flow.

    States:
      CALL_IDLE       — no call in progress
      CALL_OUTGOING   — we initiated, waiting for peer to accept
      CALL_INCOMING   — MQTT ring received, waiting for tap to accept/reject
      CALL_CONNECTED  — audio streaming active in both directions
      CALL_ENDED      — call finished, cleaning up

    Outgoing call flow:
      1. Tap badge -> SR opens -> say "Call Support"
      2. onSrEvent() maps SR command ID -> contacts index -> initiateCall("Support")
      3. initiateCall() looks up IP -> mqttPublishCallSignal(targetID, "ring")
      4. Peer taps to answer -> mqttPublishCallSignal(ourID, "accept") received
      5. startAudioStream(peerIP) -> CALL_CONNECTED, LED green

    Incoming call flow:
      1. MQTT receives ring on combadge/call/<OurBadgeID>
      2. handleIncomingCallSignal() -> state = CALL_INCOMING
         LED_2 amber, SOUND_RING plays
      3. Tap badge -> acceptCall() -> mqttPublishCallSignal(callerID, "accept")
                                   -> startAudioStream(callerIP) -> CALL_CONNECTED
         Hold badge -> rejectCall() -> mqttPublishCallSignal(callerID, "reject")

    Ending a call:
      Tap badge while CALL_CONNECTED -> endCall(true)
      endCall() -> mqttPublishCallSignal(peerID, "end")
                -> stopAudioStream()
                -> state = CALL_IDLE, LEDs off
*/

// Forward declarations for sound functions defined in sounds.ino (compiles after this file).
// Enum value must match the definition in sounds.ino exactly.
enum SoundId : uint8_t;          // forward-declare the enum type
extern void playSoundAsync(SoundId sound);

// If sounds.ino defines SOUND_RING as a plain #define rather than an enum,
// comment out the enum line above and uncomment this instead:
// #define SOUND_RING  2         // match the value in sounds.ino



// ==================== Call State ====================
enum CallState {
  CALL_IDLE,
  CALL_OUTGOING,
  CALL_INCOMING,
  CALL_CONNECTED,
  CALL_ENDED
};

volatile CallState callState = CALL_IDLE;

// Details of the current/pending call
String callPeerBadgeID = "";
String callPeerIP      = "";
String callPeerName    = "";

// Timeout for outgoing ring — if peer doesn't answer in 30s, cancel
#define CALL_RING_TIMEOUT_MS 30000
unsigned long callRingStartTime = 0;

// ==================== Outgoing call ====================

// Called when SR recognises a contact name while call_badge == true
// name must match a contacts[] entry (case-insensitive)
void initiateCall(const String& name) {
  if (callState != CALL_IDLE) {
    debugln("CallManager: already in a call.");
    return;
  }

  int idx = getContactByName(name);   // contacts.ino
  if (idx < 0) {
    debug("CallManager: contact not found: ");
    debugln(name);
    //LED_2_ERROR;
    call_badge = false;
    return;
  }

  callPeerBadgeID = String(contacts[idx].badgeID);
  callPeerIP      = String(contacts[idx].ip);
  callPeerName    = String(contacts[idx].name);

  if (callPeerIP.isEmpty()) {
    debug("CallManager: no IP for contact: ");
    debugln(callPeerName);
    //LED_2_ERROR;
    call_badge = false;
    return;
  }

  debug("CallManager: ringing ");
  debugln(callPeerName);

  callState         = CALL_OUTGOING;
  callRingStartTime = millis();

  LED_2_OUTGOING;   // green = dialling out

  // Send ring signal to peer via MQTT
  mqttPublishCallSignal(callPeerBadgeID, "ring");
}

// ==================== Incoming call ====================

// Called by mqttCallback() in mqtt_presence.ino when a call signal arrives
void handleIncomingCallSignal(const String& payload) {
  // Parse action, from, ip from payload JSON
  // Simple string parsing — avoids another ArduinoJson allocation here

  auto extractField = [](const String & json, const String & key) -> String {
    String search = "\"" + key + "\":\"";
    int start = json.indexOf(search);
    if (start < 0) return "";
    start += search.length();
    int end = json.indexOf("\"", start);
    if (end < start) return "";
    return json.substring(start, end);
  };

  String action   = extractField(payload, "action");
  String fromID   = extractField(payload, "from");
  String fromIP   = extractField(payload, "ip");

  debug("CallManager: signal action=");
  debug(action);
  debug(" from=");
  debugln(fromID);

  // ---- Incoming ring ----
  if (action == "ring") {
    if (callState != CALL_IDLE) {
      // Already on a call — auto-reject
      mqttPublishCallSignal(fromID, "reject");
      return;
    }

    callPeerBadgeID = fromID;
    callPeerIP      = fromIP;
    callState       = CALL_INCOMING;

    // Look up name from contacts if known
    int idx = getContactByBadgeID(fromID);
    callPeerName = (idx >= 0) ? String(contacts[idx].name) : fromID;

    debug("CallManager: incoming call from ");
    debugln(callPeerName);

    // Amber LED to show incoming call
    LED_2_INCOMING;

    // Play ringtone so user knows a call is arriving
    playSoundAsync(SOUND_RING);

    return;
  }

  // ---- Peer accepted our outgoing call ----
  if (action == "accept" && callState == CALL_OUTGOING) {
    debugln("CallManager: peer accepted - starting audio.");
    callState = CALL_CONNECTED;
    startAudioStream(callPeerIP);   // audio_stream.ino
    LED_2_OK;   // green = connected
    return;
  }

  // ---- Peer rejected our outgoing call ----
  if (action == "reject" && callState == CALL_OUTGOING) {
    debugln("CallManager: peer rejected call.");
    callState  = CALL_IDLE;
    call_badge = false;
    //LED_2_ERROR;
    return;
  }

  // ---- Peer ended the connected call ----
  if (action == "end" && callState == CALL_CONNECTED) {
    debugln("CallManager: peer ended call.");
    endCall(false);   // don't send "end" back — they already sent it
    return;
  }
}

// ==================== Accept / Reject / End ====================

void acceptCall() {
  if (callState != CALL_INCOMING) return;

  debugln("CallManager: accepting incoming call.");
  callState = CALL_CONNECTED;

  mqttPublishCallSignal(callPeerBadgeID, "accept");
  startAudioStream(callPeerIP);
  LED_2_OK;   // green = connected
}

void rejectCall() {
  if (callState != CALL_INCOMING) return;

  debugln("CallManager: rejecting incoming call.");
  mqttPublishCallSignal(callPeerBadgeID, "reject");
  callState  = CALL_IDLE;
  call_badge = false;
  LED_2_OFF;
}

// sendSignal=true when we are ending the call, false when peer already sent "end"
void endCall(bool sendSignal) {
  if (callState == CALL_IDLE) return;

  debugln("CallManager: ending call.");

  if (sendSignal && !callPeerBadgeID.isEmpty()) {
    mqttPublishCallSignal(callPeerBadgeID, "end");
  }

  stopAudioStream();   // audio_stream.ino

  callState       = CALL_IDLE;
  callPeerBadgeID = "";
  callPeerIP      = "";
  callPeerName    = "";
  call_badge      = false;
  call_phone      = false;

  LED_2_OFF;
  debugln("CallManager: call ended, back to idle.");
}

// ==================== Loop handler ====================
// Call from zzloop.ino every iteration

void callManagerLoop() {

  // ---- Outgoing ring timeout ----
  if (callState == CALL_OUTGOING) {
    if (millis() - callRingStartTime > CALL_RING_TIMEOUT_MS) {
      debugln("CallManager: outgoing call timed out - no answer.");
      mqttPublishCallSignal(callPeerBadgeID, "end");
      callState  = CALL_IDLE;
      call_badge = false;
      //LED_2_WAIT;
    }
    return;
  }

  // ---- call_badge flag — generic "Call Badge" utterance ----
  // This fires when the user says the generic phrase "Call Badge" (SR_CMD_CALL_BADGE).
  // With dynamic contact commands registered, users should instead say
  // "Call <name>" directly (e.g. "Call Support"), which routes straight to
  // initiateCall() via onSrEvent without going through this flag at all.
  //
  // Kept here as a fallback: if someone says the generic command and there is
  // exactly one contact, just call them.  With multiple contacts it is ambiguous,
  // so we flash the LED and clear the flag.
  if (call_badge && callState == CALL_IDLE) {
    if (contactCount == 1) {
      debugln("CallManager: generic 'Call Badge' — calling only contact.");
      initiateCall(String(contacts[0].name));
    } else if (contactCount > 1) {
      debugln("CallManager: 'Call Badge' is ambiguous — say 'Call <name>' instead.");
      no();   // flash red to signal ambiguity
      call_badge = false;
    } else {
      debugln("CallManager: no contacts loaded.");
      call_badge = false;
    }
  }
}

// ==================== Setup ====================
void callManagerSetup() {
  callState       = CALL_IDLE;
  callPeerBadgeID = "";
  callPeerIP      = "";
  callPeerName    = "";
  debugln("CallManager: ready.");
}


//EOF
