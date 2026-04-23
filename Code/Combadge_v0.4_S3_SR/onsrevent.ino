//         Test voids
//===================================================================
void yes() {
  rgbLedWrite(RGB_BUILTIN, 0, 10, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 0, 10, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
}

void no() {
  rgbLedWrite(RGB_BUILTIN, 10, 0, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 10, 0, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
}

//===================================================================

void onSrEvent(sr_event_t event, int command_id, int phrase_id) {
  switch (event) {
    case SR_EVENT_TIMEOUT:
      // seems to be about 5 seconds timeout
      ESP_SR.setMode(SR_MODE_OFF);
      LED_2_OFF;
      break;

    case SR_EVENT_COMMAND:

      // ---- Dynamic contact commands ----
      // IDs >= SR_CMD_CONTACT_BASE map directly to contacts[] by index.
      if (command_id >= SR_CMD_CONTACT_BASE) {
        int contactIdx = command_id - SR_CMD_CONTACT_BASE;
        if (contactIdx < contactCount) {
          debug("SR: calling contact: ");
          debugln(contacts[contactIdx].name);
          // Pass the contact name directly — NOT the full "Call <n>" display string
          initiateCall(String(contacts[contactIdx].name));
        } else {
          debugln("SR: contact index out of range!");
          no();
        }
        ESP_SR.setMode(SR_MODE_OFF);
        LED_2_OFF;
        break;
      }

      // ---- Fixed commands ----
      switch (command_id) {
        case SR_CMD_ZERO:                     number_0(); break;
        case SR_CMD_ONE:                      number_1(); break;
        case SR_CMD_TWO:                      number_2(); break;
        case SR_CMD_THREE:                    number_3(); break;
        case SR_CMD_FOUR:                     number_4(); break;
        case SR_CMD_FIVE:                     number_5(); break;
        case SR_CMD_SIX:                      number_6(); break;
        case SR_CMD_SEVEN:                    number_7(); break;
        case SR_CMD_EIGHT:                    number_8(); break;
        case SR_CMD_NINE:                     number_9(); break;
        // SR_CMD_CALL_BADGE kept as a fallback: calls first contact
        case SR_CMD_CALL_BADGE:               call_badge = true; break;
        case SR_CMD_CALL_PHONE:               call_phone = true; break;
        case SR_CMD_BEGIN_RECORDING:          record(); break;
        case SR_CMD_OPEN_CONFIG:              smartConfig(); break;
        case SR_CMD_CLOSE_CONFIG:             ESP.restart(); break;
        case SR_CMD_OPEN_FILES:               start_filebrowser(); break;
        case SR_CMD_CLOSE_FILES:              ESP.restart(); break;
        case SR_CMD_UPDATE_WIFI:              smartConfig(); break;
        case SR_CMD_ENABLE_WIFI:              ESP.restart(); break;
        case SR_CMD_DISABLE_WIFI:             WiFi.mode(WIFI_OFF); break;
        case SR_CMD_ENABLE_SIM:               yes(); break;
        case SR_CMD_DISABLE_SIM:              no(); break;

        // ---- Hot-reload contacts from SD and rebuild SR command table ----
        case SR_CMD_UPDATE_CONTACTS: {
          debugln("SR: Update Contacts — reloading from SD...");
          // Must stop SR before rebuilding its command table
          ESP_SR.setMode(SR_MODE_OFF);
          loadContacts();
          printContacts();
          buildSrCommands();
          // Re-initialise ESP-SR with the new table
          ESP_SR.begin(i2s, sr_commands_dynamic, sr_commands_count, SR_CHANNELS_MONO, SR_MODE_OFF);
          debugln("SR: contacts reloaded, SR command table rebuilt.");
          yes();
          LED_2_OFF;
          return;   // skip the setMode/LED at the bottom — already done
        }

        default:                              no(); debugln("Unknown Command!"); break;
      }
      ESP_SR.setMode(SR_MODE_OFF);  // turn off SR after each command
      LED_2_OFF;
      break;
  }
}
