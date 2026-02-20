
const char* wifiAPname = AP_NAME;
const char* wifiAPpassword = AP_PASSWORD;

WiFiManager wifiManager;

WiFiManagerParameter hostname_parameter;
WiFiManagerParameter timeToSwitchOff_parameter;
WiFiManagerParameter custom_numMinsBefore_parameter;
WiFiManagerParameter custom_numMinsAfter_parameter;
WiFiManagerParameter custom_numSecsFromGMT_parameter;
WiFiManagerParameter custom_boolDaylighSavingTime_parameter;

boolean wifiDidConnectEvent = false;

void setupWiFi()
{
  DebugPrintf(PSTR("Starting WiFi\n"));

  wifiDidConnectEvent = false;

  WiFi.mode(WIFI_STA);

  setupExtraParameters();

#ifdef DEBUG
  wifiManager.setDebugOutput(true);
#else
  wifiManager.setDebugOutput(false);
#endif

  DebugPrintf(PSTR("Connecting WiFi\n"));
  bool res = wifiManager.autoConnect(wifiAPname, wifiAPpassword);
  if (!res) 
  {
    DebugPrintf(PSTR("Failed to connect, resetting wifi settings and restarting in AP mode"));
    wifiManager.resetSettings();
    ESP.restart();
  }
  else
  {
    DebugPrintf(PSTR("Connected to WiFi\n"));
    wifiDidConnectEvent = true;
  }  
}

void startWiFiServices()
{
  DebugPrintf(PSTR("Starting services\n"));

  setupServices();
  startServices();
}

void stopWiFiServices() 
{
  DebugPrintf(PSTR("Stopping services\n"));

  stopServices();
}

void resetWiFiSettings()
{
  DebugPrintf(PSTR("Reset WiFi settings to defaults."));
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
  const char* custom_time_str = "<br/><label for='timeToSwitchOff'>Time to switch off (0:00-23:59): &nbsp;</label><input type='time' name='timeToSwitchOff'>";
  new (&timeToSwitchOff_parameter) WiFiManagerParameter(custom_time_str); // custom html input
  wifiManager.addParameter(&timeToSwitchOff_parameter);

  // randomMinutesBefore
  const char* custom_numMinsBefore_str = "<br/><label for='randomMinutesBefore'>Random minutes before (0-127):&nbsp;</label><input type='number' name='randomMinutesBefore' min=0 max=127>";
  new (&custom_numMinsBefore_parameter) WiFiManagerParameter(custom_numMinsBefore_str); // custom html input
  wifiManager.addParameter(&custom_numMinsBefore_parameter);

  // randomMinutessAfter
  const char* custom_numMinsAfter_str = "<br/><label for='randomMinutesAfter'>Random minutes after (0-127):&nbsp;</label><input type='number' name='randomMinutesAfter' min=0 max=127>";
  new (&custom_numMinsAfter_parameter) WiFiManagerParameter(custom_numMinsAfter_str); // custom html input
  wifiManager.addParameter(&custom_numMinsAfter_parameter);

  // secondsFromGMT
  const char* custom_numSecsFromGMT_str = "<br/><label for='secondsFromGMT'>Seconds that your time is from GMT (-43200-43200):&nbsp;</label><input type='number' name='secondsFromGMT' min=-43200 max=43200>";
  new (&custom_numSecsFromGMT_parameter) WiFiManagerParameter(custom_numSecsFromGMT_str); // custom html input
  wifiManager.addParameter(&custom_numSecsFromGMT_parameter);

  // daylightSavingTime
  const char* custom_boolDaylighSavingTime_str = "<br/><label for='daylightSavingTime'>Check if daylight saving time (summer time) is in effect:&nbsp;</label><input type='checkbox' name='daylightSavingTime' value='daylightSavingTime'>";
  new (&custom_boolDaylighSavingTime_parameter) WiFiManagerParameter(custom_boolDaylighSavingTime_str); // custom html input
  wifiManager.addParameter(&custom_boolDaylighSavingTime_parameter);

  // Callback
  
  wifiManager.setSaveParamsCallback(saveParamsCallback);
}

void saveParamsCallback()
{
  DebugPrintf(PSTR("WiFi parameters saved"));

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

  if (wifiManager.server->hasArg("secondsFromGMT"))
  {
    if ((wifiManager.server->arg("secondsFromGMT") == ""))
    {
      settings.secondsFromGMT = 0;
    }
    else
    {
     settings.secondsFromGMT = (uint32_t)atoi(wifiManager.server->arg("secondsFromGMT").c_str());
    }

    settingsChanged = true;
  }

  if (wifiManager.server->hasArg("daylightSavingTime"))
  {
    if ((wifiManager.server->arg("daylightSavingTime") == ""))
    {
      settings.daylightSavingTime = false;
    }
    else
    {
     settings.daylightSavingTime = true;
    }

    settingsChanged = true;
  }

  if (settingsChanged == true)
  {
    DebugPrintf(PSTR("Saving new settings:\n%s"), settingsDebugString().c_str());
    saveSettings();
  }
}
