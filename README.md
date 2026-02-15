# DuskLight-ESP8266

A network-connected ESP8266 that can automatically switch a light on at dusk and back off at a set time or at dawn.

This project started as a wish to have a light automatically switched on at dusk, to give the impression that there's someone home. But it was as much a project to try out the Arduino environment and the ESP8266.

Since version 1.4.0, I have added my own sunset/sunrise calculator class (SunriseCalculator.cpp/.hpp), and a more or less proper scheduler class (EventScheduler.cpp/.hpp, EventSchedulerItem.hpp). My previous implementation was a mess, and it actually had some unexpected issues. The new impementation is far more clean and modular.

However, of course it's new code and can still have its issues. For one, I would really want to have automatic timezone settings and DST switching. But that's not there yet.

In the mean time, I have learned a lot more from the Arduino platform, and have to say that for many tasks I now switch to VSCode. Even while this is not even a large project, I find that I get into #include-hell a lot already. And also navigating through my project files is just unwieldy. For instance, there is no backwards/forwards for switching between files. Such a thing is just indispensable when developing, you always switch a lot between files.

Also, I am missing some kind of registry of the libraries that I used. Between the last time I was developing on this app and this time, I had forgotten which libraries I had used. Luckily I had written it down in this README.MD. :) But this should simply be registered in some project file or something.

Still, the great thing about Arduino is that it has libraries for just about everything out there, and that is why I keep using it. But I think a few simple improvements can make life so much easier.

Anyway, here is verson 1.4.1, which has many improvements over the earlier versions.

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
- **EventScheduler.hpp**, **EventScheduler.cpp**: The CEventScheduler class that can handle events.
- **EventSchedulerItem.hpp**: The definition of the event scheduler struct.
- **SunriseCalculator.hpp**, **SunriseCalculator.cpp**: The CSunriseCalculator class that calculates the sunrise and sunset times according to the equation given in the Wikipedia page: https://en.wikipedia.org/wiki/Sunrise_equation

**ESP8266 hardware notes**

The ESP8266 has the light switch output on GPIO4. Use it to drive a relay through an NPN transistor circuit using a BC547. Take as example this circuit: https://www.eleccircuit.com/drive-relay-by-digital-circuit/. Your own implementation may vary, there are many ways to drive a relay from a GPIO pin.

The ESP8266 also listens to a button on GPIO5. This button has two purposes:

1. Switch the light on or off manually, switching to manual mode. At the next dusk/dawn/switch-off time the light will return back to automatic mode.
2. If the button is held down while powering up or resetting the ESP8266, it starts an access-point where WiFi and a few settings can be configured (also firmware can be updated).

The first time the ESP8266 is switched on, it will automatically go into acces-point mode, where the WiFi and other settings can be configured.

**Access-point mode**

When the ESP8266 is started for the first time after firmware update, or if the ESP8266 is powered up with the button pressed, it will start in access-point mode. All previous settings will have been erased and will have to be made again..

When in access-point mode, you can connect e.g. your phone's WiFi to the access point named 'DuskLightAP', with password 'DuskLight'. This will bring up a configuration page where you can select 'Configure WiFi' and configure the DuskLight.

At the top of the page will be a list of found access point SSIDs that you can connect to.

Configuration options:

- **SSID**: The access point to which you want to connect the DuskLight.
- **Password**: The password to use for the access point.
- **Hostname**: A hostname for the DuskLight, which will be used for mDNS. Enter a unique name here. Once the DuskLight is connected to you network, it will allow you to connect to it through 'http://[hostname].local'. Default is 'http://dusklight.local'.
- **Time to switch off**: By default (if no time is set), the light will be switched off at dawn. But this is not very realistic, most people switch off their light when they go to bed. So you can enter a fixed time here at which you want the light to switch off.
- **Random minutes before**: In reality nobody switches off their light at exactly the same time each night. Some days they will go to bed earlier, other days later. Here you can enter a number from 0..255, which represents a number of minutes before the **Time to switch off**-time that you might want the light to switch off.
- **Random minutes after**: Similar to the previous setting, but now for the minutes after **Time to switch off**-time.
- **Seconds that your time is from GMT (-43200-43200)**: The number of seconds from GMT that your timezone is. I.e. CET is GMT+1 and so 3600 from GMT.
- **Check if daylight saving time (summer time) is in effect**: Checkbox, which when checked, will add 3600 to your seconds from GMT, basically switching to summer time.

The **Time to switch off**-time, and **Random minutes before** and **Random minutes after** work together to make sure that the light is not switched off at the same time every day. You can for instance set the **Time to switch off**-time to 01:00, **Random minutes before** to 30 and **Random minutes after** to 60. This means that every day the light will switch off at a random time within the interval of 01:00 - 30 minutes and 01:00 + 60 minutes. I.e. the light will switch of anywhere between 0:30 and 2:00, and every day it will be a different time.

It is also possible to update the firmware when in access-point mode. More info later.

**Normal mode**

When the Dusklight is running normally and was able to connect to the WiFi and synchronize the time with the NTP server, it will simply do its job: switching the light on and off.

To make a little more insightful to what the light is doing, it presents a web interface. Browse to http://[hostname].local (where [hostname] is the host name you entered when setting up, e.g. http://dusklight.local), and you will be presented with a page with the current time and some insight into sunrise/sunset today, and when the light was last switched on or off, and when next it will be switched on or off.

A page with more insightful information on the schedule is also provided, reachable as http://[hostname].local/debug. It will show the current schedule, and the same info on switch on/off events as in the main page.

The Dusklight also provides a simple web interface through which the light can be controlled. The web interface listens to POST requests on http://[hostname].local and supports the following commands:

POST with a parameter of 'light', which can be either 'on' or 'off' (case sensitive). This will switch the light on or off, and will set the mode to 'manual'. In 'manual' mode, the light will not switch on or off on the next scheduler event, but will instead only switch back to 'auto' mode. After that, the schedule will be active again. Basically it means that if you switch the light manually, it will stay on/off as told and skip one event. This is useful when you want to extend the time the light is on until after the next event.

POST with a parameter of 'mode'='auto'. This will switch the mode back to auto.

The main web page also provides buttons that use this POST interface to control the light from the web page.

**Used libraries**

Apart from any standard libraries that are necessary for ESP8266 Arduino (and are installed automatically), this project uses the following libraries and versions:

- NtpClientLib, Version 3.0.2-beta, by German Martin
- Time version 1.6.1, by Michael Margolis
- WiFiManager, Version 2.0.17, by Tablatronix (tzapu)
- ESPAsyncUDP. Must download as a .zip and install this manually (Sketch->Include Library->Add .ZIP library...) from: https://github.com/me-no-dev/ESPAsyncUDP


More info later...
