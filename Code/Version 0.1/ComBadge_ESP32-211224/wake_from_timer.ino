
void wake_from_timer() {

  wake_state = 2;
  Serial.println("Timer Wake");
  // Connect to Net & check for Updates from MQTT server here..

}

//EOF End-Of-File
