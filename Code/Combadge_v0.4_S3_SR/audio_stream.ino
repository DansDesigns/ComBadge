/*  audio_stream.ino
 *
 *  Bidirectional audio streaming between two badges over TCP.
 *
 *  Architecture:
 *    audioTxTask — reads mic via I2S, sends raw PCM chunks to peer's TCP server
 *    audioRxTask — accepts TCP connection from peer, writes received PCM to speaker
 *
 *  Both tasks run on Core 0 (away from ESP-SR on Core 1).
 *  ESP-SR is suspended before streaming starts and resumed on call end.
 *
 *  Audio format: 16kHz, 16-bit, mono PCM — matches I2S config in setup().
 *  No codec — raw PCM keeps latency low and CPU cost near zero.
 *  At 16kHz 16-bit mono: 32000 bytes/sec per direction (~32kbps).
 *  Typical LAN latency: 80-150ms end-to-end.
 *
 *  Ports:
 *    AUDIO_RX_PORT 3000 — this badge listens here for inbound audio from peer
 *    Tx connects to peer's AUDIO_RX_PORT
 *
 *  Call manager controls streaming via:
 *    startAudioStream(peerIP)  — launches both tasks, suspends ESP-SR
 *    stopAudioStream()         — signals tasks to stop, resumes ESP-SR
 */

#define AUDIO_RX_PORT     3000
#define AUDIO_BUF_SIZE    512    // bytes per I2S read/write — ~16ms of audio at 16kHz 16-bit
#define AUDIO_TX_TIMEOUT  5000   // ms to wait for peer TCP connection before giving up

// ==================== State ====================
volatile bool audioStreamActive = false;
static String audioPeerIP = "";

NetworkServer audioServer(AUDIO_RX_PORT);

// ==================== TX Task — mic → peer ====================
void audioTxTask(void* pvParams) {
  debugln("AudioTX: task started.");

  int16_t buf[AUDIO_BUF_SIZE / 2];   // 16-bit samples
  size_t  bytesRead = 0;

  // Connect to peer's audio receive port
  NetworkClient txClient;
  unsigned long connectStart = millis();

  debug("AudioTX: connecting to ");
  debug(audioPeerIP);
  debug(":");
  debugln(AUDIO_RX_PORT);

  while (!txClient.connect(audioPeerIP.c_str(), AUDIO_RX_PORT)) {
    if (!audioStreamActive || millis() - connectStart > AUDIO_TX_TIMEOUT) {
      debugln("AudioTX: failed to connect to peer. Stopping.");
      audioStreamActive = false;
      vTaskDelete(NULL);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  debugln("AudioTX: connected to peer.");
  //LED_1_CONNECTED;

  while (audioStreamActive && txClient.connected()) {
    // Read from I2S mic
    i2s.readBytes((char*)buf, AUDIO_BUF_SIZE);

    // Send raw PCM to peer
    if (txClient.write((uint8_t*)buf, AUDIO_BUF_SIZE) == 0) {
      debugln("AudioTX: write failed — peer disconnected.");
      break;
    }
  }

  txClient.stop();
  debugln("AudioTX: task ended.");
  audioStreamActive = false;
  vTaskDelete(NULL);
}

// ==================== RX Task — peer → speaker ====================
void audioRxTask(void* pvParams) {
  debugln("AudioRX: task started, listening on port 3000.");

  audioServer.begin();

  // Wait for peer to connect
  NetworkClient rxClient;
  unsigned long waitStart = millis();

  while (!rxClient) {
    rxClient = audioServer.accept();
    if (!audioStreamActive || millis() - waitStart > AUDIO_TX_TIMEOUT + 2000) {
      debugln("AudioRX: no peer connected in time. Stopping.");
      audioStreamActive = false;
      vTaskDelete(NULL);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  debugln("AudioRX: peer connected.");

  uint8_t buf[AUDIO_BUF_SIZE];

  while (audioStreamActive && rxClient.connected()) {
    int available = rxClient.available();
    if (available >= AUDIO_BUF_SIZE) {
      rxClient.read(buf, AUDIO_BUF_SIZE);
      // Write raw PCM to I2S DAC / MAX98357 speaker
      i2s.write((const uint8_t*)buf, AUDIO_BUF_SIZE);
    } else {
      vTaskDelay(1 / portTICK_PERIOD_MS);   // yield while waiting for data
    }
  }

  rxClient.stop();
  debugln("AudioRX: task ended.");
  audioStreamActive = false;
  vTaskDelete(NULL);
}

// ==================== Public API ====================

void startAudioStream(const String& peerIP) {
  if (audioStreamActive) {
    debugln("AudioStream: already active.");
    return;
  }

  audioPeerIP      = peerIP;
  audioStreamActive = true;

  // Suspend ESP-SR so it stops consuming I2S mic — we take over directly
  ESP_SR.setMode(SR_MODE_OFF);  // Switch to OFF mode
  debugln("AudioStream: ESP-SR suspended.");

  // Launch both tasks on Core 0 with generous stack (audio buffers are large)
  xTaskCreatePinnedToCore(audioTxTask, "audioTx", 8192, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(audioRxTask, "audioRx", 8192, NULL, 2, NULL, 0);

  debugln("AudioStream: TX and RX tasks started.");
  //LED_1_4G_CONNECTED;
}

void stopAudioStream() {
  if (!audioStreamActive) return;

  audioStreamActive = false;
  debugln("AudioStream: stopping...");

  // Give tasks a moment to notice the flag and exit cleanly
  delay(300);

  // Resume speech recognition
  ESP_SR.setMode(SR_MODE_COMMAND);  // Switch back to Command detection
  debugln("AudioStream: ESP-SR resumed.");

  //LED_1_OK;
}

bool isAudioStreamActive() {
  return audioStreamActive;
}


//EOF
