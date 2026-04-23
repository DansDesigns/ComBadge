void mtch_setup()
{

  // the operating mode (MODE)
  byte data;

  data = readRegister(0x05);
  debug("Current MODE: ");
  Serial.println(data, BIN);

  //~~~~~~~~~~~~~~~~~~~~~~~~ Mode Selection: (0x05) ~~~~~~~~~~~~~~~~~~~~~~~~

  /*
      MODE<3:0>: Touch Decoding mode bits
    Binary        = Mode     = Normal Number = Hex
    0000 = Standby        0     0x00
    0001 = Gesture        1     0x01
    0010 = Touch only     2     0x02
    0011 = Full (touch and gesture)   3     0x03

      Convert last bit below, to binary above
  */

  // ~~~~~~~~~~ writeRegister DOES NOT WORK, MOVED TO SINGLE SETUP~~~~~~~~~~

  // Set mode to Touch only
  //writeRegister(0x05, 0x02);

  // Set mode to Gesture only
  // writeRegister(0x05, 0x01);

  Wire.beginTransmission(ADDR);
  Wire.write(0x05); // register to read
  Wire.write(0x01); // Data to Write
  byte busStatus = Wire.endTransmission();
  if (busStatus != 0x00)
  {
    debugln(busStatus);
  }

  // Set mode to Full Gesture + Touch (Default Mode)
  //writeRegister(0x05, 0x03);

  //~~~~~~~~~~~~~~~~~~~~~~~~ Read Mode: (0x05) ~~~~~~~~~~~~~~~~~~~~~~~~
  data = readRegister(0x05);
  debug("New MODE: ");
  Serial.println(data, BIN);

  //~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x20 NUMBEROFXCHANNELS ~~~~~~~~~~~~~~~~~~~~~~~~
  data = readRegister(0x20);
  debug("NUMBEROFXCHANNELS: ");
  debugln(data);

  //~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x21 NUMBEROFYCHANNELS ~~~~~~~~~~~~~~~~~~~~~~~~
  data = readRegister(0x21);
  debug("NUMBEROFYCHANNELS: ");
  debugln(data);

  //~~~~~~~~~~~~~~~~~~~~~~~~ Write Registers 0x20, 0x21 ~~~~~~~~~~~~~~~~~~~~~~~~
  //~~~~~~~~~~~~~~~~~~~~ Set no. Channels X & Y to 0x07 & 0x06 (Default 0x09, 0x06) ~~~~~~~~~~~~~~~~~~~~~~~~
  //  writeRegister(0x20, 0x07);
  //  writeRegister(0x21, 0x06);
  // pcb design has 5x5
  //~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x20 NUMBEROFXCHANNELS ~~~~~~~~~~~~~~~~~~~~~~~~
  data = readRegister(0x20);
  debug("NUMBEROFXCHANNELS: ");
  debugln(data);

  //~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x21 NUMBEROFYCHANNELS ~~~~~~~~~~~~~~~~~~~~~~~~
  data = readRegister(0x21);
  debug("NUMBEROFYCHANNELS: ");
  debugln(data);
}

void mtch_run()
{
  byte data;

  /*
    // ~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x10 TOUCHSTATE ~~~~~~~~~~~~~~~~~~~~~~~~
    data = readRegister(0x10);
    debug("TOUCHSTATE: ");
    debuglnf(data,BIN);

    // ~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x11 Touch X ~~~~~~~~~~~~~~~~~~~~~~~~
    data = readRegister(0x11);
    debug("TOUCHX: ");
    debuglnf(data,BIN);

    // ~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x21  Touch Y ~~~~~~~~~~~~~~~~~~~~~~~~
    data = readRegister(0x12);
    debug("TOUCHY: ");
    debuglnf(data,BIN);

    // ~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x13 TOUCHLSB ~~~~~~~~~~~~~~~~~~~~~~~~
    data = readRegister(0x13);
    debug("TOUCHLSB: ");
    debuglnf(data,BIN);
  */

  // ~~~~~~~~~~~~~~~~~~~~~~~~ Read Register 0x14 GESTURESTATE ~~~~~~~~~~~~~~~~~~~~~~~~
  //  data = readRegister(0x14);
  //  debug("GESTURESTATE: ");
  //  debugln(data);

  if (digitalRead(MTCH6102_Interupt_Pin) == LOW)
  {
    data = readRegister(0x14);
    debug("GESTURESTATE: ");
    debugln(data);
    debugln("~~~~~~~~~~~~~~");
  }
  /* ~~~~~~~~~~~~~~~~~~~~~~~~ 0x14 GESTURESTATE ~~~~~~~~~~~~~~~~~~~~~~~~

       GESTURESTATE<7:0>:
    0x00 No Gesture Present
    0x10 Single Click
    0x11 Click and Hold
    0x20 Double Click
    0x31 Down Swipe
    0x32 Down Swipe and Hold
    0x41 Right Swipe
    0x42 Right Swipe and Hold
    0x51 Up Swipe
    0x52 Up Swipe and Hold
    0x61 Left Swipe
    0x62 Left Swipe and Hold

    When a gesture is performed, the gesture ID will be placed in GESTURESTATE, and the GES bit of the
    TOUCHSTATE register will be set. Both of these items are cleared after reading the GESTURESTATE register.
  */



  // ~~~~~~~~~~~~~~~~~~~~~~~~ Read All Registers 0x80 -> 0x8E ~~~~~~~~~~~~~~~~~~~~~~~~
  /*
    debug("SENSORVALUE_RX <i>: ");
    for (byte i = 0x80; i < 0x8E; i++) {
       data = readRegister(i);

       //debug(i, HEX);
      //debug(" = ");
      debugf(data,BIN);
      debug(", ");
    }
    debugln();
  */

  /*
      data = readRegister(0x80);
      debug("SENSORVALUE<RX0>: ");
      debuglnf(data,BIN);
      data = readRegister(0x8D);
      debug("SENSORVALUE<RX13>: ");
      debuglnf(data,BIN);
      data = readRegister(0x86);
      debug("SENSORVALUE<RX6>: ");
      debuglnf(data,BIN);
  */
  delay(500);
}



//EOF
