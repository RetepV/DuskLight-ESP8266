
const char* wifiAPname = AP_NAME;
const char* wifiAPpassword = AP_PASSWORD;

WiFiManager wifiManager;

WiFiManagerParameter hostname_parameter;
WiFiManagerParameter timeToSwitchOff_parameter;
WiFiManagerParameter custom_numMinsBefore_parameter;
WiFiManagerParameter custom_numMinsAfter_parameter;

boolean wifiConnected = false;

void setupWiFi()
{
  DebugPrintf("Starting WiFi\n");

  WiFi.mode(WIFI_STA);

  setupExtraParameters();

#ifdef DEBUG
  wifiManager.setDebugOutput(true);
#else
  wifiManager.setDebugOutput(false);
#endif

  bool res = wifiManager.autoConnect(wifiAPname, wifiAPpassword);
  if (!res) 
  {
    DebugPrintf("Failed to connect, reset wifi settings and restart in AP mode");
    wifiManager.resetSettings();
    ESP.restart();
  }
  else
  {
    DebugPrintf("Connected to WiFi\n");
    wifiConnected = true;
  }  
}

void startWiFiServices()
{
  DebugPrintf("Starting services\n");

  setupServices();
  startServices();
}

void stopWiFiServices() 
{
  DebugPrintf("Stopping services\n");

  stopServices();
}

void resetWiFiSettings()
{
  wifiManager.resetSettings();
}

// Using WiFiManager to set up extra parameters.

// hostname: The hostname for mDNS.
// timeToSwitchOff: The time to switch off the light. If this is larger than 1440, we will switch off the light at
//                  dawn. Otherwise we will switch off the light at this time. We can add a random component to make
//                  the light randomly switch off, + or - seconds from timeToSwitchOff.
// randomSecondsBefore: A number of seconds that the light might be switched off before timeToSwitchOff.
// randomSecondsAfter: A number of seconds that the light might be switched off after timeToSwitchOff.

uint16_t stringToMinutes(String timeString)
{
  struct tm timeComponents;
  strptime(timeString.c_str(), "%H:%M", &timeComponents);
  return (timeComponents.tm_hour * 60) + timeComponents.tm_min;
}

void setupExtraParameters()
{
  // hostname
  new (&hostname_parameter) WiFiManagerParameter("hostname", "Hostname", "", HOSTNAME_MAXLENGTH, DEFAULT_MDNS_PLACEHOLDER_STRING);
  wifiManager.addParameter(&hostname_parameter);

  // timeToSwitchOff
  const char* custom_time_str = "<br/><label for='timeToSwitchOff'>Time to switch off: &nbsp;</label><input type='time' name='timeToSwitchOff'>";
  new (&timeToSwitchOff_parameter) WiFiManagerParameter(custom_time_str); // custom html input
  wifiManager.addParameter(&timeToSwitchOff_parameter);

  // randomMinutesBefore
  const char* custom_numMinsBefore_str = "<br/><label for='randomMinutesBefore'>Random minutes before:&nbsp;</label><input type='number' name='randomMinutesBefore' min=0 max=255>";
  new (&custom_numMinsBefore_parameter) WiFiManagerParameter(custom_numMinsBefore_str); // custom html input
  wifiManager.addParameter(&custom_numMinsBefore_parameter);

  // randomSecondsAfter
  const char* custom_numMinsAfter_str = "<br/><label for='randomMinutesAfter'>Random minutes after:&nbsp;</label><input type='number' name='randomMinutesAfter' min=0 max=255>";
  new (&custom_numMinsAfter_parameter) WiFiManagerParameter(custom_numMinsAfter_str); // custom html input
  wifiManager.addParameter(&custom_numMinsAfter_parameter);

  // Callback
  
  wifiManager.setSaveParamsCallback(saveParamsCallback);
}

void saveParamsCallback()
{
  DebugPrintf("WiFi parameters saved");

  bool settingsChanged = false;

  // hostname
  
  if (wifiManager.server->hasArg("hostname") && (wifiManager.server->arg("hostname") != ""))
  {
    sprintf(settings.hostname, "%s", wifiManager.server->arg("hostname").c_str());
    settingsChanged = true;
  }

  if (wifiManager.server->hasArg("timeToSwitchOff"))
  {
    if ((wifiManager.server->arg("timeToSwitchOff") == ""))
    {
      settings.timeToSwitchOff = MINUTES_IN_DAY + 1;
    }
    else
    {
     settings.timeToSwitchOff = stringToMinutes(wifiManager.server->arg("timeToSwitchOff"));
    }

    settingsChanged = true;
  }

  if (wifiManager.server->hasArg("randomMinutesBefore"))
  {
    settings.randomMinutesBefore = (uint8_t)atoi(wifiManager.server->arg("randomMinutesBefore").c_str());
    settingsChanged = true;
  }

  if (wifiManager.server->hasArg("randomMinutesAfter"))
  {
    settings.randomMinutesAfter = (uint8_t)atoi(wifiManager.server->arg("randomMinutesAfter").c_str());
    settingsChanged = true;
  }

  if (settingsChanged == true)
  {
    DebugPrintf("Saving new settings:\n%s", settingsDebugString().c_str());

    saveSettings();
  }
}
