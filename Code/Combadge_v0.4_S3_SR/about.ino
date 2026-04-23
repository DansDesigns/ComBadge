/*  ESP32 ComBadge (Voxair) v0.2 - ESP32-SR Version
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
  Confirmed Compilation Settings:

      ESP32S3 Dev Module
      USB Mode: Hardware CDC & Jtag
      USB CDC on Boot: Enabled
      Frequency: 240MHz (WiFi)
      Flash Size: 16M
      Partition Scheme: ESP32 SR (3Mb App, 7Mb Spiffs, 2.9Mb Model)
      PSRAM: OPI PSRAM
      Core 1
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

Chirp idea:
fan made chirps & ringtones placed on SD card, if none there default to built-in chirp & ring..    
make using Chrip_Generator, share in forum


 ESP32 Smart Config (ConfigAP) ONLY 2.4GHz
      
 *  MQTT-Broker (not on VPN):
 *    - each Badge has its own MQTT <BadgeID> Topic
 *    - publishing IP Address on reconnect
 *    - publish on new subscriber?
 * 
 *  Husarnet (upto 5 max connections on Free Account):
 *    - all Badges are on same VPN network - so visable to each other
 *    -
 *    
 * New Function Idea:
 *   ESP32 side (Caller/Sender/Host):
 *    - On Boot:
 *      - Connect to saved WiFi/Open Config-AP to select SSID
 *      - Connect to Husarnet VPN
 *      - MQTT Publish self IP Address
 *      - 
 *      - light-Sleep
 *      
 *    - Tap to wake
 *    - ESP32-SR interprets Command:
 *      - if Command = Call Support
 *        -
 *        
 *      - if Command = Call <Name>
 *        - search <Contact List> for <Name> & <BagdeID>
 *        - Subscribe to Receiver-<BadgeID> MQTT & obtain IP Address
 *            - Caller goes to <Receiver IP Address> /receive.html
 *            - when Caller on <Receiver IP Address> /receive.html,
 *              - Receiver GET request Caller-<BadgeID>
 *              - Caller Records 5 seconds
 *              - 
 *            - Recevier Subscribes to Caller-<BadgeID> MQTT & obtain IP Address
 *            - Receiver goes to <Caller IP Address> /sender.html
 *            - 
 *          
 *          - if failed to connect to MQTT - Error Beep/Disconnected Phone Beep
 *          
 *      - if Command = Record Memo/Record Note
 *      
 *      - if Command = 
 *      
 *      - if Command = Open Config/Show Config
 *        - Open Config-AP to select WiFi
 *      
 *      - if Command = Close Config/End Config
 *        - Close Config-AP
 *        
 *      - if Command = Open Files/Show Files
 *        - Opens Wifi File Browser
 *        
 *      - if Command = Close Files/End Files
 *        - Close Wifi File Browser
 *      
 *      - if Command = Update Contacts/Sync Contacts
 *        - delete all existing SR-Contact-Commands
 *        - extract all <Name> from Contact_List.txt & add as SR-Command
 *        - Update SR-Commands
 *      
 *      - if Command = Update Wifi/Refresh Wifi
 *        - scans SSID in range
 *        - checks Wifi_networks.txt for SSID & Password
 *        - connects using saved credntials
 *      - other commands here...
 *     
 *    - Add Contact <Name> & <BadgeID> to Contact_List.txt
 *      - via Web Interface (in-the-field add contact)
 *      - via direct file upload to Badge/SD (backup contact list)
 *      
 *    - Add WiFi to list
 *      - Using same .txt file method for Add Contact to add new SSID & Password to Badge
 *      - 
 *          
 *   
 *    
 *   
 * 
 * 
 * 
 * 
 * 
 */

 //EOF
