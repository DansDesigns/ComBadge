void wake()
{
  rgbLedWrite(RGB_BUILTIN, 0, 10, 0);
  delay(500);
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  external_ip();

  //  i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT);
  //
  //  ESP_SR.onEvent(onSrEvent);
  //  ESP_SR.begin(i2s, sr_commands, sizeof(sr_commands) / sizeof(sr_cmd_t), SR_CHANNELS_STEREO, SR_MODE_WAKEWORD);

}
