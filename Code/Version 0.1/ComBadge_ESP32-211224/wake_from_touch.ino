
void wake_from_touch() {

  //  if ((ESP_SLEEP_WAKEUP_TOUCHPAD) && (touch_counter == 0) && (wake_state == 0)) {
  //    Serial.println("Wake from Touch");
  wake_state = 1;
  touch_counter = 1;
  //  }

  wakePin = esp_sleep_get_touchpad_wakeup_status();

#if CONFIG_IDF_TARGET_ESP32
  switch (wakePin)
  {
    case 0  : Serial.println("Touch detected on GPIO 4"); break;
    case 1  : Serial.println("Touch detected on GPIO 0"); break;
    case 2  : Serial.println("Touch detected on GPIO 2 (Shift pressed)"); break;  // TouchValue1/SHIFT
    case 3  : Serial.println("Touch detected on GPIO 15"); break;
    case 4  : Serial.println("Touch detected on GPIO 13"); break;
    case 5  : Serial.println("Touch detected on GPIO 12 (Main pressed)"); break; // TouchValue0/MAIN
    case 6  : Serial.println("Touch detected on GPIO 14"); break;
    case 7  : Serial.println("Touch detected on GPIO 27 (middle pressed)"); break; // TouchValue2/MIDDLE
    case 8  : Serial.println("Touch detected on GPIO 33 (bottom pressed)"); break; // TouchValue3/BOTTOM
    case 9  : Serial.println("Touch detected on GPIO 32"); break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }

#else
  if (touchPin < TOUCH_PAD_MAX)
  {
    Serial.printf("Touch detected on GPIO %d\n", touchPin);
  }
  else
  {
    Serial.println("Wakeup not by touchpad");
  }
#endif

}

//EOF End-Of-File
