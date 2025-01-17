
void loop() {
  unsigned long currentTime = millis();
  currtouched = cap.touched();        //Call first thing to read from MPR121
  touchValue0 = touchRead(T5);    // Main Touch Wake
  touchValue1 = touchRead(T2);    // SHIFT Touch Wake
  touchValue2 = touchRead(T7);
  touchValue3 = touchRead(T8);    

  //DAC output on pin25
  //int Value = 0; //255=3.3V, 128=1.65V
  //dacWrite(DAC1, Value);

  //Serial.println(cap.touched());          // debug touch sensor
   
  
// Main Functions:

  
  if (touch_counter == 1) {       // Combadge Awake and Listening

    // Active Command Section
    //...
    //Serial.println("Listening...");
    // doop-deep
    delay(20);
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off
    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    //  Record upto 3 seconds from I2S Microphone and send to MQTT server:
  }

  
  if (touch_counter == 2) {       // Deacticate ComBadge

    tone(Tones, 1576);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    touch_counter = 0;
    //delay(20);
  }

  if (touch_counter > 3) {      // Over-Tap Protection
    touch_counter = 0;
  }

  // Internal Touch Pins:
  
  // Internal Touch 0 / Main Touch Pad
  if (touchValue0 < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 0 (IT5) Touched");   
    /* 
    // doop-deep
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    
    touch_counter++;
    */
  }
  
  // Internal Touch 1
  if (touchValue1 < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 1 (IT2) Touched");
  }
  
  // Internal Touch 2
  if (touchValue2 < threshold) {
    cycle_counter = 0;
    Serial.println("Touch 2 (IT7) Touched");
  }
  



  // MPR121 Touch Pins:
 
  if (cap.touched() & (1 << 0))
  {
    cycle_counter = 0;
    Serial.println("T0 Touched");
    //Tap
    // doop-deep
    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    delay(20);                   // Waits some time to turn off

    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
    delay(20);                   // Waits some time to turn off
    noTone(Tones);               //Turns the Tones off
    touch_counter++;
  }

  
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

// Multi-Function Key Combinations with touchValue3 being SHIFT

  if ((touchValue3 < threshold) && (cap.touched() & (1 << 2)))    // change to 6 when finished dev Top - On-Demand Config AP
  {
    cycle_counter = 0;
    Serial.println("Shift TOP Touched!");
    combadgeap();
  }

  if ((touchValue3 < threshold) && (cap.touched() & (1 << 7)))    // Middle - Mute
  {
    cycle_counter = 0;
    Serial.println("Shift MID Touched");
  }

  if ((touchValue3 < threshold) && (cap.touched() & (1 << 8)))    // Bottom - 
  {
    cycle_counter = 0;
    Serial.println("Shift BOTTOM Touched");
  }





  //Wake from Sleep Reason and Reaction:
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER : wake_from_timer(); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : wake_from_touch(); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
  }

  lasttouched = currtouched;    //Last call for Capacitive Touch Sensor
  delay(200);
  cycle_counter++;
  //Serial.print("Cycle Counter:");
  //Serial.println(cycle_counter);
  if (cycle_counter > 120) {       //20 = 5 seconds of awake time, 40 = 10 seconds, 120 = 30 seconds, 200 = 50 seconds
    cycle_counter = 0;
    lightSleep();
  }
}

//EOF End-Of-File
