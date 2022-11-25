# ComBadge mk1 ESP32-WROOM-32E, OEP3W amp, 8ohm Speaker
# last edit 25/11/22
# OTA Update from GitHub Release

from OTA_updater import OTAUpdater

def download_and_install_update_if_available():
    o = OTAUpdater('url-to-your-github-project')
    o.install_update_if_available_after_boot('wifi-ssid', 'wifi-password')

def tap():
   # when touched but not released

def double_tap():
    # when double tapped within 400ms

def hold_tap():
    # when held for 1000ms (1 second)

def release_tap():
    # when released from single tap, debounce is 50ms


def start():
    # main code goes here:
    from machine import Pin, TouchPad, deepsleep, Timer, I2S, I2C, DAC
    import utime as time
    import esp32
    from aswitch import Pushbutton

    dac = machine.DAC(25)

    magfield = esp32.hall_sensor()
    temp = esp32.raw_temperature()

    wake = Pin(14, mode=Pin.IN)
    touch = TouchPad(wake)
    touch.config(500)
    esp32.wake_on_touch(True)
    pb = Pushbutton(wake, suppress=True)



# Main code runs from this point:
    while True:
        # Play listening chirp after wake and listen for command:

        # or if the touchpin is tapped while awake, play chirp and listen:
        pb.double_func(double_tap)  # when double tapped.. (function) change toggle and colour to needed function
        pb.long_func(hold_tap)  # when held (set for 2 and 5 second commands?)
        pb.press_func(tap)  # when pressed (begin counter?)
        pb.release_func(release_tap)  # when released (single tap)

        # if (TouchPad.read(14) <50):


            #start timer here for double tap check:

            # play chirp:

            # listen for command:

        # Sleeps until next Touch-wake
        deepsleep()


# Needed for OTA updates:
def boot():
    download_and_install_update_if_available()
    start()
# Last Line of File
boot()

# End-Of-File
