
void wake_from_timer() {

  wake_state = 2;
  Serial.println("Timer Wake");
  // Check for Updates from MQTT server here..

}

//EOF End-Of-File
