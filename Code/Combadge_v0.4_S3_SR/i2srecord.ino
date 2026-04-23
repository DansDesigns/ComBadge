void record()
{
//  LED_1_WAIT;
  ESP_SR.setMode(SR_MODE_OFF);
  delay(100);

  // Raw I2S sanity check — are we getting ANY data at all?
  int16_t testBuf[64];
  size_t bytesRead = 0;
  i2s.readBytes((char*)testBuf, sizeof(testBuf));
  bool allZero = true;
  for (int i = 0; i < 64; i++) {
    if (testBuf[i] != 0) {
      allZero = false;
      break;
    }
  }
  debugln(allZero ? "I2S raw read: ALL ZERO (no mic data)" : "I2S raw read: got data OK");


  // test
  //  Reinitialise I2S in RX mode before recording
  i2s.end();
  i2s.setPins(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN);
  i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT);


  uint8_t* wav_buffer;
  size_t   wav_size;

  debugln("Starting recording...");

  // i2s.recordWAV() returns a complete buffer that ALREADY includes a
  // 44-byte WAV header. Do NOT write a separate header first, that was
  // causing a double-header bug where the first 44 bytes of audio data
  // were being interpreted as a second header by any playback software.

  wav_buffer = i2s.recordWAV(RECORD_SECONDS, &wav_size);

  debug("Received WAV buffer size: ");
  debugln(wav_size);

  if (wav_size == 0 || wav_buffer == nullptr) {
    debugln("ERROR: No audio data received from I2S.");
  //  LED_1_ERROR;
    return;
  }

  SD.remove(filename);
  file = SD.open(filename, FILE_WRITE);
  if (!file) {
    debugln("ERROR: Failed to open file for writing.");
    //LED_1_ERROR;
    return;
  }

  debugln("Writing to SD...");

  if (file.write(wav_buffer, wav_size) != wav_size) {
    debugln("ERROR: SD write incomplete.");
    file.close();
    //LED_1_ERROR;
    return;
  }

  file.close();
  debugln("Recording saved OK.");
  //LED_1_OK;

  // Restart SR when done
  //ESP_SR.begin(i2s, sr_commands, sizeof(sr_commands) / sizeof(sr_cmd_t), SR_CHANNELS_STEREO, SR_MODE_WAKEWORD);
  ESP_SR.setMode(SR_MODE_COMMAND);
}


//EOF
