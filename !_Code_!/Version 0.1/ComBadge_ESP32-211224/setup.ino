
void setup() {
  //~~~~~~~~~~~~ Hardware Init: ~~~~~~~~~~~~
  Serial.begin(115200);   //Serial on usb
  Wire.begin();  //I2C_0.begin(SDA=21 , SCL=22 , 100000 );
  pinMode(12, INPUT_PULLDOWN);          // Touch Wake Pin T5(MAIN)
  pinMode(2, INPUT_PULLDOWN);          // Touch Wake Pin T2(SHIFT)
  touchSleepWakeUpEnable(T2,threshold);
  touchSleepWakeUpEnable(T5,threshold);
  cap.begin(0x5A);                        //       I2C Address: 0x5A
  cap.setThreshholds(12, 6);
  
  //~~~~~~~~~~~~ Filesystem Init: ~~~~~~~~~~~~
  EEPROM.begin(512); //Initialasing EEPROM
  Serial.println("Reading saved EEPROM ssid");
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading saved EEPROM pass");
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  //WiFi.begin(esid.c_str(), epass.c_str());
  
 //~~~~~~~~~~~~ Sounnd Settings: ~~~~~~~~~~~~
  dac1.disable();
  pinMode(ns8002_mute, OUTPUT);         // Mute Pin for Amp
  //pinMode(Tones, OUTPUT);               //PWM Tone Output to Amp
  analogWrite(ns8002_mute, 0);
  //digitalWrite(ns8002_mute, LOW);
  
  //~~~~~~~~~~~~ OTA Update Check: ~~~~~~~~~~~~
  

  
/*
  // Boot Chime
  tone(Tones, 1076);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(20);                   // Waits some time to turn off
  tone(Tones, 1276);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(20);                   // Waits some time to turn off
  tone(Tones, 1476);          // Sounds the Tones at the frequency relative to the note C in Hz
  delay(20);                   // Waits some time to turn off
  noTone(Tones);               //Turns the Tones off
  delay(100);                   // Waits some time to turn off
  // Mute Audio Outputs
  dacWrite(DAC1, 0);
  tone(Tones, 0);
  noTone(Tones);
*/
  
  badgeid();


}

//EOF End-Of-File
