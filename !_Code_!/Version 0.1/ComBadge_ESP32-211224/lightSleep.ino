
void lightSleep() {
  Serial.println(" ");
  Serial.println("Sleeping Now..");
//  delay(100);
//  touchAttachInterrupt(12, callback, threshold);        // WAKE on Touch
//  touchAttachInterrupt(2, callback, threshold);        // WAKE on Touch
//  esp_sleep_enable_touchpad_wakeup();
//  esp_sleep_enable_timer_wakeup(20 * 60 * 100000);      // First Characters = timer: 2 = roughly 22 seconds, 10 = 1 minute?, 20 = 2 minutes..
//  touch_counter = 0;
//  wake_state = 0;
//  esp_light_sleep_start( );


#if CONFIG_IDF_TARGET_ESP32
  //Setup sleep wakeup on Touch Pad 3 + 7 (GPIO15 + GPIO 27)
  touchSleepWakeUpEnable(T2, threshold);
  touchSleepWakeUpEnable(T5, threshold);

#else //ESP32-S2 + ESP32-S3
  //Setup sleep wakeup on Touch Pad 3 (GPIO3)
  touchSleepWakeUpEnable(T3, threshold);

#endif

  //Go to sleep now
  Serial.println("Going to sleep now");
  delay(888);
  esp_light_sleep_start();    // wake up from lightsleep resumes after here..
}

//EOF End-Of-File
