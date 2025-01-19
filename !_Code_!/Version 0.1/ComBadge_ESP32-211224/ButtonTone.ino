
void ButtonTone() {
  dac1.enable();
  dac1.outputCW(876);
  //delay(20);
  //dac1.outputCW(976);
  delay(20);
  dac1.disable();
}

//
//    delay(20);
//    tone(Tones, 1876);          // Sounds the Tones at the frequency relative to the note C in Hz
//    delay(20);                   // Waits some time to turn off
//    noTone(Tones);               //Turns the Tones off
//    delay(20);                   // Waits some time to turn off
//    tone(Tones, 1976);          // Sounds the Tones at the frequency relative to the note C in Hz
//    delay(20);                   // Waits some time to turn off
//    noTone(Tones);               //Turns the Tones off
//    delay(20);                   // Waits some time to turn off

//EOF
