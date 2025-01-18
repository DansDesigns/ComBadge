

# Voxari


## "Carry the Conversation Everywhere."

* an ESP32 powered Communications Badge. Inspired by Star Trek and based on The Next Generation's ComBadge.

* No copyright infringement intended, this is purely an exercise in fun and an homage to some great Sci-Fi Technology!


* NOTE, THIS IS STILL IN DEVELOPMENT..




Discord Server:
https://discord.gg/cbAzHKdTbj

Youtube Development Channel:
https://www.youtube.com/channel/UC_F5HQEmCyoRnEubfEMa24g


# VERSION 0.2 IS IN REDESIGN BASED ON FEEDBACK FROM V0.1

Version 0.2 Changelog:
```
Added MTCH6102 multi-gesture Trackpad controller as replacement for Main Capacitive Touch Panel.
Upgraded large single electrode to custom Trackpad Array.
Upgraded side TouchPads from Internal ESP32 TouchPins to MPR121 Pins.
Upgraded ESP32-WROOM-32 to ESP32-S3-WRROM-1.
Upgraded low level Class-D Audio Amplifier to I2S MAX98357 Audio Amplifier.
Added RFM95 Lora-WiFi Adaptor for off-grid communication via Wireless Config-AP.
Added WS2812b Notification/Status RGB LED
Upgraded power converter output from 600ma to 1200ma

```

Version 0.1 errors:
```
Wake from touch stopped working with upgrade to ESP32 core v.3.x
downgraded ESP32 core in Arduino IDE to 2.0.14 however error(222) still remains.
Main Touch Panel has issues with excess charge & random activations due to physical size of the electrode..
```

## Version 0.1a:
Hardware:
```
ESP32-WROOM-32e
NS8002 Amplifier Module
8ohm Speaker (20mm x 14mm x 3mm)
Wireless Charging Transfer of Power from 2000mah Battery to ComBadge
INMP441 I2S Mic (14mm OD)
3D Printed Case with Real Electroplated Metal coating, Silver or Gold.
Potential for future design to include ability to choose graphic under a transparent Main Touch Pad.
```

## Hardware Setup
```
3D printed case (comprised of Back, Mid and Top)
with Electroplated Touch Buttons around the Mid-edge and Main Touch Pad on the Top.
Individual BADGEID etched into each Back piece. (See 3d Models Folder).

Manufactured PCB (excludes Amplifier module, 3v3 Regulator module, Speakers and Wireless Charging PCBS) (See PCB Folder)

Class-D/Class-AB Amplifier module https://www.ebay.co.uk/itm/204126443946

3v3 Regulator https://shop.pimoroni.com/products/ap3429a-3-3v-buck-converter-breakout-3-3v-output-1-2a-max?variant=32173899546707

Wireless Charging PCB https://www.aliexpress.com/item/1005003173949105.html

INMP441 Microphone https://www.ebay.co.uk/itm/284801985119

8 Ohm Speaker https://www.ebay.co.uk/itm/194934853031

```

## Code & Libraries
```
The Voice Recognition is done by Vosk, an offline (locally run) Python Speech Recognition toolkit.
- Supports 20+ languages and dialects - English, Indian English, German, French, Spanish, Portuguese,
Chinese, Russian, Turkish, Vietnamese, Italian, Dutch, Catalan, Arabic, Greek, Farsi, Filipino,
Ukrainian, Kazakh, Swedish, Japanese, Esperanto, Hindi, Czech, Polish, Uzbek, Korean. With more to come.

Arduino Tones library is used to generate the different beeps.

Capacitive Touch is powered by the MPR121 chip aswell as using the ESP32's onboard Capacitive Touch Pins
- Multi-Tap Button Combinations make for more control options with less physical areas. (See Manual for more info)

I2S Microphone Streaming was modified from here:
- https://github.com/pschatzmann/arduino-audio-tools/tree/main

Wifi Manager was taken from here:
- https://github.com/ozbotics/WIFIMANAGER-ESP32/tree/master

OTA Updater modified from:
- https://www.youtube.com/watch?v=o4Qw0CDzO7E&pp=ygUYZXNwMzIgYXJkdWlubyBvdGEgZ2l0aHVi


```

## Installation
```
If building yourself, Flash the OTA_Installer.ino to the ESP32 via Arduino IDE.

On reboot the ComBadge OTA Update will search GitHub for the latest Release,
it automatically Downloads, Installs and Reboots the ComBadge.

Manual checking for update is possible by typing "U" into the Arduino IDE Serial Monitor.

If need to change WIFI SSID, press the CONFIG Button combination and connect to the
"ComBadge_Setup_AP" WIFI Access Point from a wifi enabled device, following the on screen instructions.
```

## Usage
```
Single-Tap (Tap and Release to register)

Single tap on the Capacitive Touch Surface wakes from deep_sleep,
Plays the Activation Beep and activates "Listening Mode".

In "Listening Mode" there are several Spoken Commands:
- Call <NAME>
- Time, Date, Weather etc
- Record
- Add Contact
- More to come

CALL <NAME>
- Calls the requested NAME, aslong as that name is stored locally with a coresponding <BADGEID>.

TIME, DATE etc
- Say Time, Date, Weather or other commands to recieve that data back.

RECORD
- Begins recording from the microphone & writes to a file until a Second-Tap is registered.

ADD CONTACT
- Spoken Command to add new contact (stored locally). Server asks for <NAME> and <BADGEID>


```

## Progress/ToDo
```
WIP = In Progress | TBS = To Be Started | R&D = Research needed | DONE = Finished dev & in usable state

- Make simple OTA_Base.ino starter file to run OTA updater, pulling latest Release to flash full working code. - DONE

- Save SSID data to local storage for use across power states (after dismount/reboot). - DONE

- Bluetooth Classic HFP (Hands Free Protocol) for Mobile Phone pass-thru - R&D, TBS

- Bluetooth PAN for internet connectivity instead of WIFI - R&D, TBS

- "Add Contact" Functionality - WIP

- Experiment with Transparent Electroconductive Spray Paint for customisable Top graphics. - R&D, WIP

- OTA Updating from GitHub - DONE

- LORAWIFI for custom comms network with large range and low power. - R&D, TBS

- more that isn't listed here..

```

```
## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

```

Check out this great project,
with more of a walkie-talkie take on the ComBadge,
here: https://github.com/Pippadi/combadge


