
void setup() {
  Serial.begin(115200);   //Serial on usb
  Wire.begin();  //I2C_0.begin(SDA=21 , SCL=22 , 100000 );

  pinMode(ns8002_mute, OUTPUT);         // Mute Pin for Amp
  pinMode(Tones, OUTPUT);               //PWM Tone Output to Amp
  pinMode(12, INPUT_PULLDOWN);          // Touch Wake Pin (MAIN)
  pinMode(33, INPUT_PULLDOWN);          // Touch Wake Pin (SHIFT)
  digitalWrite(ns8002_mute, LOW);
  cap.begin(0x5A);                        //       I2C Address: 0x5A
  cap.setThreshholds(12, 6);
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
#endif

          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
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
