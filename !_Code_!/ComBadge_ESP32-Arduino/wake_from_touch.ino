
void wake_from_touch() {

//  if ((ESP_SLEEP_WAKEUP_TOUCHPAD) && (touch_counter == 0) && (wake_state == 0)) {
//    Serial.println("Wake from Touch");
//    wake_state = 1;
//    touch_counter = 1;
//  }

  wakePin = esp_sleep_get_touchpad_wakeup_status();

  switch (wakePin)
  {
    case 0  : Serial.println("Touch detected on GPIO 4"); break;
    case 1  : Serial.println("Touch detected on GPIO 0"); break;
    case 2  : Serial.println("Touch detected on GPIO 2"); break;  // TouchValue1
    case 3  : Serial.println("Touch detected on GPIO 15"); break;
    case 4  : Serial.println("Touch detected on GPIO 13"); break;
    case 5  : Serial.println("Touch detected on GPIO 12"); break; // TouchValue0/Main
    case 6  : Serial.println("Touch detected on GPIO 14"); break;
    case 7  : Serial.println("Touch detected on GPIO 27"); break; // TouchValue2
    case 8  : Serial.println("Touch detected on GPIO 33"); break; // TouchValue3/Shift
    case 9  : Serial.println("Touch detected on GPIO 32"); break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }


}

//EOF End-Of-File
