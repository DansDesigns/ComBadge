
//On-Demand Config AP:
//define your default Savable Values here. if there are different values in config.json, they are overwritten.
//These appear in the On-Demand Config AP as editable text fields.
//char mqtt_server[40];
//char mqtt_port[6] = "8080";
//char pubTopic[] = "Badges";

void launchWeb()
{
  //Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    //Serial.println("WiFi connected");
  //Serial.print("Local IP: ");
  //Serial.println(WiFi.localIP());
  //Serial.print("SoftAP IP: ");
  //Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  //Serial.println("Server started");
}


void combadgeap()
{
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";                    // start list
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<div><li><a href='#p' onclick='c(this)'>";
    //st += "<li>";               // list
    st += WiFi.SSID(i);         // ssid for list number
    //st += " (";
    //st += WiFi.RSSI(i);         // signal strength for ssid
    //st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li></a>&nbsp;</div>";
  }
  st += "</ol>";              // end list
  delay(100);
  WiFi.softAP("Setup_AP", "Tintin10");  // SSID, Password
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  //Serial.println("over");

}


//~~~~~~~~~~~~~~~~~~ OLD AP CODE ~~~~~~~
//{
//  // The extra parameters to be configured (can be either global or just in the setup)
//  // After connecting, parameter.getValue() will get you the configured value
//  // id/name placeholder/prompt default length
//  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
//  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
//  
//  //WiFiManager
//  //Local intialization. Once its business is done, there is no need to keep it around
//  WiFiManager wifiManager;
//  
//  //set config save notify callback
//  wifiManager.setSaveConfigCallback(saveConfigCallback);
//  
//  //add all your parameters here
//  wifiManager.addParameter(&custom_mqtt_server);
//  wifiManager.addParameter(&custom_mqtt_port);
//
//  if (!wifiManager.startConfigPortal("ComBadge_Setup", "password")) {
//    Serial.println("failed to connect and hit timeout");
//    delay(3000);
//    //reset and try again, or maybe put it to deep sleep
//    ESP.restart();
//    delay(5000);
//  }
//
//  //if you get here you have connected to the WiFi
//  badgeid();
//  Serial.println("Connection Established...");
//  Serial.print("CONNECTED TO: ");
//  Serial.println(WiFi.SSID());
//  Serial.print("IP ADDRESS: ");
//  Serial.println(WiFi.localIP());
//  Serial.print("SIGNAL: ");
//  Serial.print(WiFi.RSSI());
//
//
//  //read updated parameters
//  strcpy(mqtt_server, custom_mqtt_server.getValue());
//  strcpy(mqtt_port, custom_mqtt_port.getValue());
//
//    //save the custom parameters to FS
//  if (shouldSaveConfig) {
//    Serial.println("saving config");
// #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
//    DynamicJsonDocument json(1024);
//#else
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& json = jsonBuffer.createObject();
//#endif
//    json["mqtt_server"] = mqtt_server;
//    json["mqtt_port"] = mqtt_port;
//
//    File configFile = SPIFFS.open("/config.json", "w");
//    if (!configFile) {
//      Serial.println("failed to open config file for writing");
//    }
//
//#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
//    serializeJson(json, Serial);
//    serializeJson(json, configFile);
//#else
//    json.printTo(Serial);
//    json.printTo(configFile);
//#endif
//    configFile.close();
//    //end save
//  }
//  
//}

//EOF End-Of-File
