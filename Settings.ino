
SettingsType_t settings;

void setupSettings()
{
  EEPROM.begin(sizeof(settings));
}

void startSettings()
{
  fetchSettings();
}

void endSettings()
{
  saveSettings();
  EEPROM.end();
}

void fetchSettings()
{
  unsigned int startAddress = 0;

  DebugPrintf(PSTR("EEPROM signature: %02X %02X %02X\n"), EEPROM.read(0), EEPROM.read(1), EEPROM.read(2));

  if ((EEPROM.read(0) == 0x55) && (EEPROM.read(1) == 0xAA) && (EEPROM.read(2) == 0x00))
  {
    DebugPrintf(PSTR("Found settings, loading\n"));
    EEPROM.get(startAddress, settings);
    DebugPrintf(PSTR("Loaded settings:\n%s"), settingsDebugString().c_str());
  }
  else
  {
    // There is no signature: settings are cleared, settings version has been updated, or settings are just random garbage.
    // Write current (default) settings as new settings. Nicer would be to provide a way of upgrading the settings, but
    // for now the best thing to do is to just restart the device with the button held down so that it goes into AP
    // mode, and reprogram everything.
    DebugPrintf(PSTR("No settings found, writing defaults\n"));
    EEPROM.put(startAddress, settings);
    EEPROM.commit();
  }
}

void saveSettings()
{
  DebugPrintf(PSTR("Saving new settings:\n%s"), settingsDebugString().c_str());
  unsigned int startAddress = 0;
  EEPROM.put(startAddress, settings);
  EEPROM.commit();
  DebugPrintf(PSTR("Settings saved"));
}

void eraseSettings()
{
  for (unsigned int index = 0; index < sizeof(settings); index++) {
    EEPROM.write(index, 0);
  }
  EEPROM.commit();
  DebugPrintf(PSTR("Settings erased"));
}

String settingsDebugString()
{
  String debugString = "";

  debugString += "hostname             : " + String(settings.hostname) + "\n";
  debugString += "timeToSwitchOff      : " + String(settings.timeToSwitchOff) + "\n";
  debugString += "randomSecondsBefore  : " + String(settings.randomMinutesBefore) + "\n";
  debugString += "randomSecondsAfter   : " + String(settings.randomMinutesAfter) + "\n";
  debugString += "secondsFromGMT       : " + String(settings.secondsFromGMT) + "\n";
  debugString += "daylightSavingTime   : ";
  debugString += (settings.daylightSavingTime ? "on (summer time)" : "off (winter time)");
  debugString += "\n";

  return debugString;
}
