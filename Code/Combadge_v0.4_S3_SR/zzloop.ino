void loop()
{

  byte data;

  // ~~~~~~~~~~~~~~~~~~~~~~~~ MTCH6102 Gesture Handler ~~~~~~~~~~~~~~~~~~~~~~~~
  if (digitalRead(MTCH6102_Interupt_Pin) == LOW)
  {
    //LED_1_READY;
    data = readRegister(0x14);   // GESTURESTATE register
    //debug("GESTURESTATE: 0x");
    //Serial.println(data, HEX);

    switch (data)
    {
      case 0x10:  // Single tap
        debugln("Single tap");
        if (callState == CALL_INCOMING) {
          acceptCall();                               // tap to answer
        } else if (callState == CALL_CONNECTED || callState == CALL_OUTGOING) {
          endCall(true);                              // tap while in call to hang up
        } else {
          srListenRequested = true;                   // idle: open SR listen window
          LED_2_LISTENING;
          playSoundAsync(SOUND_CHIRP);
        }
        break;
      case 0x11:  // Click and hold
        debugln("Click and hold");
        if (callState == CALL_INCOMING) {
          rejectCall();                               // hold to reject
        }
        break;
      case 0x20: debugln("Double click");         break;
      case 0x31: debugln("Down swipe");           break;
      case 0x32: debugln("Down swipe and hold");  break;
      case 0x41: debugln("Right swipe");          break;
      case 0x42: debugln("Right swipe and hold"); break;
      case 0x51: debugln("Up swipe");             break;
      case 0x52: debugln("Up swipe and hold");    break;
      case 0x61: debugln("Left swipe");           break;
      case 0x62: debugln("Left swipe and hold");  smartConfig(); break;
      default:  break;
    }
  }

  if (srListenRequested) {
    //ESP_SR.onEvent(onSrEvent);
    ESP_SR.setMode(SR_MODE_COMMAND);
    srListenRequested = false;
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~ Serial Debug Commands ~~~~~~~~~~~~~~~~~~~~~~~~
  if (Serial.available() > 0)
  {
    Mode = Serial.read();
    switch (Mode)
    {
      case 'R':
        debugln("--- Manual: Restart ---");
        ESP.restart();
        break;

      case 'W':
        debugln("--- Manual: WiFi reconnect ---");
        keepWifiAlive();
        break;

      case 'E':
        debugln("--- Manual: External IP ---");
        external_ip();
        break;

      case 'S':
        debugln("--- Manual: Light sleep ---");
        esp_light_sleep_start();
        break;

      case 'M':
        debugln("--- Manual: MTCH6102 setup ---");
        mtch_setup();
        break;

      case 'F':
        debugln("--- Manual: File browser ---");
        start_filebrowser();
        break;

      case 'C':
        debugln("--- Manual: SmartConfig ---");
        smartConfig();
        break;

      case '1':
        debugln("--- Manual: Chirp ---");
        playSoundAsync(SOUND_CHIRP);
        break;

      case '2':
        debugln("--- Manual: Ring ---");
        playSoundAsync(SOUND_RING);
        break;

      case '3':
        debugln("--- Manual: Record ---");
        record();
        break;

      // ---- Provisioning commands ----
      // K<base64key> — provision a new AES decryption key into NVS
      // Example: send  Kbase64encodedkeyhere=  via serial monitor
      case 'K': {
          String keyB64 = "";
          unsigned long t = millis();
          while (millis() - t < 2000) {   // read rest of line within 2 seconds
            if (Serial.available()) keyB64 += (char)Serial.read();
          }
          keyB64.trim();
          if (keyB64.length() > 0) {
            if (provisionAESKey(keyB64)) {
              debugln("Provisioning: AES key saved. Reload join code with 'H'.");
            }
          } else {
            debugln("Provisioning: no key data received after 'K'.");
          }
          break;
        }

      // H — reload Husarnet join code from SD (after updating /husarnet.key)
      case 'H':
        debugln("--- Manual: Reload Husarnet join code ---");
        if (loadJoinCode(husarnetJoinCode)) {
          debugln("Join code reloaded. Restart to reconnect Husarnet.");
        } else {
          debugln("Failed to reload join code.");
        }
        break;

      default:
        break;
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~ WiFi Credential Reset ~~~~~~~~~~~~~~~~~~~~~~~~
  // Hold boot button for 3 seconds to wipe saved SSID/password and restart
  rst_millis = millis();
  while (digitalRead(WiFi_rst) == LOW)
  {
    if (millis() - rst_millis >= 3000)
    {
      debugln("Clearing WiFi credentials and restarting...");
      writeStringToFlash("", 0);    // clear SSID
      writeStringToFlash("", 40);   // clear password
      delay(500);
      ESP.restart();
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~ MQTT (actualy UDP) ~~~~~~~~~~~~~~~~~~~~~~~~
  // Handles reconnection, incoming presence updates, and call signals.
  mqttLoop();

  // ~~~~~~~~~~~~~~~~~~~~~~~~ Call Manager ~~~~~~~~~~~~~~~~~~~~~~~~
  // Monitors call_badge flag, manages call state machine, ring timeouts.
  callManagerLoop();

  // ~~~~~~~~~~~~~~~~~~~~~~~~ HTTP Server ~~~~~~~~~~~~~~~~~~~~~~~~
  // Must be polled every loop - handles GET / (badge info JSON) and GET /ping.
  // Will also handle POST /call and GET /stream once call_manager.ino is added.
  client_connected();

  // ~~~~~~~~~~~~~~~~~~~~~~~~ File Manager ~~~~~~~~~~~~~~~~~~~~~~~~
  if (filemanager_active)
  {
    filemgr.handleClient();
  }

  //  sleep_millis = millis();
  //  if (millis() - sleep_millis >= 10000)
  //  {
  //    esp_light_sleep_start();
  //  }


  // Reduced from 100ms. a 100ms loop delay would cause audio glitches
  // once streaming tasks are added. Tasks use their own blocking delays.
  delay(10);
 
}


//EOF
