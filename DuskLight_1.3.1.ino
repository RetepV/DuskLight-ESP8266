
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

  Serial.printf("\n");
  Serial.printf("DuskLight V1.3.1\n");

  // Initialise pins.

  pinMode(gpioLed, OUTPUT);
  digitalWrite(gpioLed, 0);

  pinMode(gpioLight, OUTPUT);
  digitalWrite(gpioLight, 1);

  pinMode(gpioButton, INPUT_PULLUP);

  // Initial setup

  setupSettings();
  // If button is pressed during power up, erase settings. Device will go into AP mode.  
  if (digitalRead(gpioButton) == LOW)
  {
    DebugPrintf("Resetting settings and starting AP\n");
    eraseSettings();
    resetWiFiSettings();
  }
  startSettings();

  // Further setup is done when WiFi connects.
  setupWiFi();
  startWiFi();
}

void loop()
{
  // Handle button, using a crude debouncing algorithm. Basically sample HIGH/LOW and LOW/HIGH transitions every 20ms.
  static int lastButtonCheck = 0;
  if ((millis() - lastButtonCheck) > 20)
  {
    lastButtonCheck = millis();
    if ((previousButtonState == HIGH) && (digitalRead(gpioButton) == LOW))
    {
      setLightMode(LightModeManual);
      if (lightIsOn())
      {
        DebugPrintf("Light mode manual off (button press)\n");
        switchLightOff();
      }
      else
      {
        DebugPrintf("Light mode manual on (button press)\n");
        switchLightOn();
      }
      previousButtonState = LOW;
    }
    else if ((previousButtonState == LOW) && (digitalRead(gpioButton) == HIGH))
    {
      previousButtonState = HIGH;
    }
  }

  // Update MDNS state

  MDNS.update();

  // Handle web server requests
  
  webServer.handleClient();

  // Handle NTP events
  
  if (shouldProcessNTPEvent)
  {
    processNTPSyncEvent();
    shouldProcessNTPEvent = false;
  }

  if (timeHasBeenSynced)
  {
    // Check light state every 60 seconds
    EXECUTE_PERIODICALLY(lastTimeKeeperCheck, 60 * 1000, checkForLightActions();)
  }
  else
  {
    EXECUTE_PERIODICALLY(lastFlashLedCheck, 250, flashLed();)    
  }
    
  delay(0);
}
