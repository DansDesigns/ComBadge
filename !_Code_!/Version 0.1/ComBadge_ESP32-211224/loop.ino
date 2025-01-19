
void loop() {
  unsigned long currentTime = millis();
  currtouched = cap.touched();        //Call first thing to read from MPR121
  TouchShift = touchRead(T2);    // SHIFT - Touch Wake
  TouchMain = touchRead(T5);    // MAIN - Touch Wake
  TouchMiddle = touchRead(T7);    // Middle -
  TouchBottom = touchRead(T8);    // Bottom -
  // MPR121_T0  = Top

  //DAC output on pin25
  //int Value = 0; //255=3.3V, 128=1.65V
  //dacWrite(DAC1, Value);

  //Serial.println(cap.touched());          // debug touch sensor


  // Main Functions:


  if (touch_counter == 1) {       // Combadge Awake and Listening

    //~~~~~~~~~~~~~~~~~~~~~ Active Command Section ~~~~~~~~~~~~~~~~~~~~~
    //...
    //Serial.println("Listening...");
    // doop-deep
    //ButtonTone();

    dac1.enable();
    dac1.outputCW(876);
    //delay(20);
    //dac1.outputCW(976);
    delay(20);
    dac1.disable();
    
    //  Record upto 3 seconds from I2S Microphone and send to MQTT server:
    
    touch_counter = 0;
    
  }

  //~~~~~~~~~~~~~~~~~~~~~ Deactivate ComBadge ~~~~~~~~~~~~~~~~~~~~~
  if (touch_counter == 2) {

    dac1.enable();
    dac1.outputCW(576);
    delay(20);
    dac1.disable();
    touch_counter = 0;
    //delay(20);
  }
  //~~~~~~~~~~~~~~~~~~~~~ Over-Tap Protection ~~~~~~~~~~~~~~~~~~~~~
  if (touch_counter > 3) {
    touch_counter = 0;
  }

  //~~~~~~~~~~~~~~~~~~~~~ Internal Touch Pins: ~~~~~~~~~~~~~~~~~~~~~

  // Internal Touch 5 / Main Touch Pad
  if (TouchMain < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 0 (IT5) Touched");
    // doop-deep
    //ButtonTone();
    touch_counter++;

  }

  //  //~~~~~~~~~~~~~~~~~~~~~ Internal Touch 1~~~~~~~~~~~~~~~~~~~~~
  //  if (touchValue1 < threshold) {
  //    cycle_counter = 0;
  //    Serial.println("Touch 1 (IT2) Touched");
  //  }
  //
  //  //~~~~~~~~~~~~~~~~~~~~~ Internal Touch 2~~~~~~~~~~~~~~~~~~~~~
  //  if (touchValue2 < threshold) {
  //    cycle_counter = 0;
  //    Serial.println("Touch 2 (IT7) Touched");
  //  }
  //



  //~~~~~~~~~~~~~~~~~~~~~ MPR121 Touch Pins: ~~~~~~~~~~~~~~~~~~~~~

  //  if (cap.touched() & (1 << 0))
  //  {
  //    cycle_counter = 0;
  //    Serial.println("T0 Touched");
  //    //Tap
  //    // doop-deep
  //    ButtonTone();
  //    touch_counter++;
  //  }


  if (cap.touched() & (1 << 5))
  {

  }

  if (cap.touched() & (1 << 6))
  {

  }

  if (cap.touched() & (1 << 7))
  {

  }

  if (cap.touched() & (1 << 8))
  {

  }

  //~~~~~~~~~~~~~~~~~~~~~ Multi-Function Key Combinations with Internal-Touch-Pins (IT2 being SHIFT) ~~~~~~~~~~~~~~~~~~~~~


  if ((TouchShift < threshold) && (cap.touched() & (1 << 0)))    // Top - On-Demand Config AP
  {
    cycle_counter = 0;
    Serial.println("Shift + TOP Touched!");
    combadgeap();
  }

  if ((TouchShift < threshold) && (TouchMiddle < threshold))    // Middle - Mute
  {
    cycle_counter = 0;
    Serial.println("Shift + MID Touched");
  }

  if ((TouchShift < threshold) && (TouchBottom < threshold))    // Bottom -
  {
    cycle_counter = 0;
    Serial.println("Shift + BOTTOM Touched");
  }



  //~~~~~~~~~~~~~~~~~~~~~ Multi-Function Key Combinations with MPR121 (IT2 being SHIFT) ~~~~~~~~~~~~~~~~~~~~~

  if ((TouchShift < threshold) && (cap.touched() & (1 << 0)))    // Top - On-Demand Config AP
  {
    cycle_counter = 0;
    Serial.println("Shift + TOP Touched!");
    combadgeap();
  }

  if ((TouchShift < threshold) && (cap.touched() & (1 << 7)))    // Middle - Mute
  {
    cycle_counter = 0;
    Serial.println("Shift + MID Touched");
  }

  if ((TouchShift < threshold) && (cap.touched() & (1 << 8)))    // Bottom -
  {
    cycle_counter = 0;
    Serial.println("Shift + BOTTOM Touched");
  }

  //~~~~~~~~~~~~~~~~~~~~~ Wake from Sleep Reason and Reaction: ~~~~~~~~~~~~~~~~~~~~~
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER : wake_from_timer(); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : wake_from_touch(); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    case ESP_SLEEP_WAKEUP_UNDEFINED : null ; break; // for debuggin use: Serial.println("UNKNOWN WakeUp Cause") -in place of "null"
    case ESP_SLEEP_WAKEUP_ALL : Serial.println("UNKNOWN-ALL WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_EXT0 : wake_from_lid_open() ; break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("EXT1 WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_GPIO : Serial.println("GPIO WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_UART : Serial.println("UART WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_WIFI : Serial.println("WIFI WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_BT : Serial.println("BT WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_COCPU : Serial.println("COCPU WakeUp Cause") ; break;
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG : Serial.println("COCPU_TRAP_TRIG WakeUp Cause") ; break;
      //    case ESP_SLEEP_WAKEUP_UNDEFINED : Serial.println("UNKNOWN WakeUp Cause") ; break;
  }

  lasttouched = currtouched;    //Last call for Capacitive Touch Sensor
  delay(200);
  cycle_counter++;
  //Serial.print("Cycle Counter:");
  //Serial.println(cycle_counter);

  //~~~~~~~~~~~~~~~~~~~~~ Wake Timer ~~~~~~~~~~~~~~~~~~~~~
  if (cycle_counter > 20) {       //20 = 5 seconds of awake time, 40 = 10 seconds, 120 = 30 seconds, 200 = 50 seconds
    cycle_counter = 0;
    lightSleep();
  }
}

//EOF End-Of-File
