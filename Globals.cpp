
#include <arduino.h>
#include "Globals.h"

// Global variables

int previousButtonState = HIGH;

// Callbacks for when WiFi states change.

void setupServices()
{
  DebugPrintf("Setup TimeKeeper\n");
  setupTimeKeeper();
  DebugPrintf("Setup MDNS\n");
  setupMDNS();
  DebugPrintf("Setup NTP\n");
  setupNTP();
  DebugPrintf("Setup webserver\n");
  setupWebserver();
}

void startServices()
{
  DebugPrintf("Start MDNS\n");
  startMDNS();
  DebugPrintf("Start NTP\n");
  startNTP();
  DebugPrintf("Start webserver\n");
  startWebserver();

  digitalWrite(gpioLed, 1);
}

void stopServices()
{
  DebugPrintf("Stop MDNS\n");
  stopMDNS();
  DebugPrintf("Stop NTP\n");
  stopNTP();
  DebugPrintf("Stop webserver\n");
  stopWebserver();
}
