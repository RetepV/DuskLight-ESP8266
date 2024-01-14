
#include <arduino.h>
#include "Globals.h"

// Global variables

int previousButtonState = HIGH;

// Callbacks for when WiFi states change.

void setupServices()
{
  setupMDNS();
  setupNTP();
  setupWebserver();
}

void startServices()
{
  startMDNS();
  startNTP();
  startWebserver();

  digitalWrite(gpioLed, 1);
}

void stopServices()
{
  stopMDNS();
  stopNTP();
  stopWebserver();
}
