
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

  DebugPrintf("EEPROM signature: %02X %02X %02X\n", EEPROM.read(0), EEPROM.read(1), EEPROM.read(2));

  if ((EEPROM.read(0) == 0x55) && (EEPROM.read(1) == 0xAA) && (EEPROM.read(2) == 0x00))
  {
    DebugPrintf("Found settings, loading\n");
    EEPROM.get(startAddress, settings);
  }
  else
  {
    // There is no signature, settings are cleared or random garbage. Write current (default) settings as new settings.
    DebugPrintf("No settings found, writing defaults\n");
    EEPROM.put(startAddress, settings);
    EEPROM.commit();
  }
}

void saveSettings()
{
  unsigned int startAddress = 0;
  EEPROM.put(startAddress, settings);
  EEPROM.commit();
  DebugPrintf("Settings saved");
}

void eraseSettings()
{
  for (unsigned int index = 0; index < sizeof(settings); index++) {
    EEPROM.write(index, 0);
  }
  EEPROM.commit();
  DebugPrintf("Settings erased");
}

String settingsDebugString()
{
  String debugString = "";

  debugString += "hostname             : " + String(settings.hostname) + "\n";
  debugString += "timeToSwitchOff      : " + String(settings.timeToSwitchOff) + "\n";
  debugString += "randomSecondsBefore  : " + String(settings.randomMinutesBefore) + "\n";
  debugString += "randomSecondsAfter   : " + String(settings.randomMinutesAfter) + "\n";

  return debugString;
}
