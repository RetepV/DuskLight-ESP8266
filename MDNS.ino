
void setupMDNS()
{
  DebugPrintf(PSTR("Setup mDNS\n"));
}

void startMDNS() 
{
  DebugPrintf(PSTR("Start mDNS with hostname \"%s\"\n"), settings.hostname);
  MDNS.begin(settings.hostname);
}

void stopMDNS()
{
  DebugPrintf(PSTR("Stop mDNS\n"));  
}
