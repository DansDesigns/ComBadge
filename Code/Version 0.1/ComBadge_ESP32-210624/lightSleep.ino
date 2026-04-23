
void lightSleep() {
  Serial.println("Sleeping Now..");
  delay(100);
  touchAttachInterrupt(12, callback, threshold);        // WAKE on Touch
  touchAttachInterrupt(33, callback, threshold);        // WAKE on Touch
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_timer_wakeup(20 * 60 * 100000);      // 2 = roughly 22 seconds? 10 = 1 minute?
  touch_counter = 0;
  wake_state = 0;
  esp_light_sleep_start( );
}

//EOF End-Of-File
