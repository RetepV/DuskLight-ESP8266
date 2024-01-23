# DuskLight-ESP8266

A network-connected ESP8266 that can automatically switch a light on at dusk and back off at a set time or at dawn.

This project started as a wish to have a light automatically switched on at dusk, to give the impression that there's someone home. But it was as much a project to try out the Arduino environment and the ESP8266.

This code has had multiple increments, but never had a rewrite. I am releasing this mostly because I spent time to structure the Arduino program in a nicer way than putting all code into one bloated .ino file. For sure it can be improved, and I will improve it myself over time.

**Project structure**

- **DuskLight_ESP8266.ino**: The main .ino file
- **Globals.h**: Contains defines, macros, includes typedefs, constants and forward declarations
- **Global.cpp**: Contains some global variables and functions
- **MDNS.ino**: MDNS functionality
- **NTP.ino**: NTP functionality
- **Settings.ino**: Settings functionality
- **TimeKeeper.ino**: Functionality to calculate the times when the light should be switched on and off
- **WebServer.ino**: Functionality for a webserver to gain some insights and to be able to switch the light on/off manually or through some automation script
- **WiFi.ino**: Functionality related to WiFi and WiFiManager

The code does its work, but I won't improve it much anymore. Therefore releasing it into the public domain.

**ESP8266 hardware notes**

The ESP8266 has the light switch output on GPIO4. Use it to drive a relay through an NPN transistor circuit using a BC547. Take as example this circuit: https://www.eleccircuit.com/drive-relay-by-digital-circuit/. Your own implementation may vary, there are many ways to drive a relay from a GPIO pin.

The ESP8266 also listens to a button on GPIO5. This button has two purposes:

1. Switch the light on or off manually, switching to manual mode. At the next dusk/dawn/switch-off time the light will return back to automatic mode.
2. If the button is held down while powering up or resetting the ESP8266, it starts an access-point where WiFi and a few settings can be configured (also firmware can be updated).

The first time the ESP8266 is switched on, it will automatically go into acces-point mode, where the WiFi and other settings can be configured.

**Settings**

When the ESP8266 is started for the first time after firmware update, or if the ESP8266 is powered up with the button pressed, it will start in access-point mode.

When in access-point mode, you can connect e.g. your phone's WiFi to the access point named 'DuskLightAP', with password 'DuskLight'.

This will bring up a configuration page where you can select 'Configure WiFi' and configure the DuskLight.

At the top of the page will be a list of found access point SSIDs that you can connect to.

Configuration options:

- **SSID**: The access point to which you want to connect the DuskLight
- **Password**: The password to use for the access point
- **Hostname**: A hostname for the DuskLight, which will be used for mDNS. Enter a unique name here. Once the DuskLight is connected to you network, it will allow you to connect to it through 'http://[hostname].local'. Default is 'http://dusklighthost.local'.
- **Time to switch off**: By default (if no time is set), the light will be switched off at dawn. But this is not very realistic, most people switch off their light when they go to bed. So you can enter a fixed time here at which you want the light to switch off.
- **Random minutes before**: In reality nobody switches off their light at exactly the same time each night. Some days they will go to bed earlier, other days later. Here you can enter a number from 0..255, which represents a number of minutes before the **Time to switch off**-time that you might want the light to switch off.
- **Random minutes after**: Similar to the previous setting, but now for the minutes after **Time to switch off**-time.

The **Time to switch off**-time, and **Random minutes before** and **Random minutes after** work together to make sure that the light is not switched off at the same time every day. You can for instance set the **Time to switch off**-time to 01:00, **Random minutes before** to 30 and **Random minutes after** to 60. This means that every day the light will switch off at a random time within the interval of 01:00 - 30 minutes and 01:00 + 60 minutes. I.e. the light will switch of anywhere between 0:30 and 2:00, and every day it will be a different time.

It is also possible to update the firmware when in access-point mode. More info later.

Finally, when in access-point mode, you can open a page with a bunch of information about the ESP8266.

**Libraries**

Apart from any standard libraries that are necessary for ESP8266 Arduino (and are installed automatically), this project uses the following libraries and versions:

- Dusk2Dawn, Version 1.01, by DM Kishi
- NtpClientLib, Version 3.0.2-beta, by German Martin
- Time Version, 1.6.1, by Paul Stoffregen
- WiFiManager, Version 2.0.16-rc.2, by Tablatronix (tzapu)

More info later...
