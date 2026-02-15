
// Include the one and only Globals.h

#include "Globals.h"

// Helpers

void flashLed()
{
  static bool ledIsOn = true;
  
  digitalWrite(gpioLed, ledIsOn ? 1 : 0);

  ledIsOn = !ledIsOn;
}

// App lifetime

void setup() 
{
  // Init serial and print logon message

  Serial.begin(115200);

  DebugPrintf("\n");
  DebugPrintf("%s %s\n", APPLICATION_NAME, APPLICATION_VERSION);

  // Initialise pins.

  pinMode(gpioLed, OUTPUT);
  digitalWrite(gpioLed, 0);

  pinMode(gpioLight, OUTPUT);
  digitalWrite(gpioLight, 1);

  pinMode(gpioButton, INPUT_PULLUP);

  // Initial setup

  DebugPrintf("Setup settings\n");
  setupSettings();

  // If the button is pressed during power up, erase all settings. Device will go into AP mode.  
  if (digitalRead(gpioButton) == LOW)
  {
    DebugPrintf("Resetting settings and starting in AP mode.\n");
    eraseSettings();
    resetWiFiSettings();
  }

  DebugPrintf("Fetch settings\n");
  startSettings();

  // Further setup (starting of services) is done when WiFi connects.
  DebugPrintf("Setup WiFi\n");
  setupWiFi();
}

void handleButtonStateChanges()
{
    if ((previousButtonState == HIGH) && (digitalRead(gpioButton) == LOW))
    {
      setLightMode(LightModeManual);
      if (lightIsOn())
      {
        DebugPrintf("Light mode manual off (button press)\n");
        switchLight(false);
      }
      else
      {
        DebugPrintf("Light mode manual on (button press)\n");
        switchLight(true);
      }
      previousButtonState = LOW;
    }
    else if ((previousButtonState == LOW) && (digitalRead(gpioButton) == HIGH))
    {
      previousButtonState = HIGH;
    }
}

void handleEventStateChanges()
{
  checkForEvents();
}

void loop()
{

  // // Handle button, using a crude debouncing algorithm. Basically sample HIGH/LOW and LOW/HIGH transitions every 20ms.
  // static int lastButtonCheck = 0;
  // if ((millis() - lastButtonCheck) > 20)
  // {
  //   lastButtonCheck = millis();
  //   handleButtonStateChanges();
  // }

  // Check button state changes once every 20ms, crude debouncing.
  EXECUTE_PERIODICALLY(lastButtonCheck, 20, handleButtonStateChanges();)

  // Handle WiFi connected
  if (wifiDidConnectEvent)
  {
    wifiDidConnectEvent = false;
    startWiFiServices();
  }

  // Handle NTP events
  if (shouldProcessNTPEvent)
  {
    processNTPSyncEvent();
    shouldProcessNTPEvent = false;
  }

  // Handle web server requests
  webServer.handleClient();

  // Update MDNS state
  MDNS.update();

  // Handle scheduler
  if (timeHasBeenSynced)
  {
    // Check for events every 10 seconds.
    EXECUTE_PERIODICALLY(lastTimeKeeperCheck, 10 * 1000, handleEventStateChanges();)
  }
  else
  {
    EXECUTE_PERIODICALLY(lastFlashLedCheck, 250, flashLed();)    
  }
    
  delay(50);
}
