# DuskLight-ESP8266

A network-connected ESP8266 that can automatically switch a light on at dusk and back of at a set time or at dawn.

This project started as a wish to have a light automatically switched on at dusk. But it was as much a project to try out the Arduino environment and to try out an ESP8266. It has had multiple increments, but never had a rewrite.

I am releasing this mostly as an example of how an Arduino application could be structured in a nicer way than putting all code into one bloated .ino file.

**Project structure**

- DuskLight_1.3.1.ino: The main .ino file
- Globals.h: Contains defines, macros, includes typedefs, constants and forward declarations
- Global.cpp: Contains some global variables and functions
- MDNS.ino: MDNS functionality
- NTP.ino: NTP functionality
- Settings.ino: Settings functionality
- TimeKeeper.ino: Functionality to calculate the times when the light should be switched on and off
- WebServer.ino: Functionality for a webserver to gain some insights and to be able to switch the light on/off manually or through some automation script
- WiFi.ino: Functionality related to WiFi and WiFiManager

The code does it's work, but I won't improve it anymore. Therefore releasng it into the public domain. This code will be the basis for a next version that will be a fully schedulable switch, having a much more versatile schedule.

The ESP8266 has the switch output on GPIO4. In my light, it drives a 5V relay through an NPN transistor circuit with a BC547. Take for example this circuit: https://www.eleccircuit.com/drive-relay-by-digital-circuit/

**ESP8266 hardware notes**

The ESP8266 also listens on GPIO5 for a button. This button has two purposes:

1. Switch the light on or off manually. At the next dusk/dawn/switch-off time it will return back to automatic mode.
2. If the button is held down while powering up or resetting the ESP8266, it starts an access-point where WiFi and a few settings can be configured.

The first time the ESP8266 is switched on, it will automatically go into acces-point mode, where the WiFi and other settings can be configured.

**Libraries**

Apart from any standard libraries that are necessary for ESP8266 Arduino (and are installed automatically), this project currently uses the following libraries and versions:

- Dusk2Dawn, Version 1.01, by DM Kishi
- NtpClientLib, Version 3.0.2-beta, by German Martin
- Time Version, 1.6.1, by Paul Stoffregen
- WiFiManager, Version 2.0.16-rc.2, by Tablatronix (tzapu)

More info later...
