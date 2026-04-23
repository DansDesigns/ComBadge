// wake_from_lid_open()
// this function should only be valid on Scicorder mini - left in for debugging on COMBadge v0.1a


void wake_from_lid_open() {

//  if ((ESP_SLEEP_WAKEUP_TOUCHPAD) && (touch_counter == 0) && (wake_state == 0)) {
//    Serial.println("Wake from Touch");
//    wake_state = 1;
//    touch_counter = 1;
//  }

  wake_state = 2;
  Serial.println("EXT0 Wake");


}

//EOF End-Of-File
