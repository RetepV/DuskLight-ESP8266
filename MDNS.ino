
void setupMDNS()
{
  DebugPrintf("Setup mDNS\n");
}

void startMDNS() 
{
  DebugPrintf("Start mDNS with hostname \"%s\"\n", settings.hostname);
  MDNS.begin(settings.hostname);
}

void stopMDNS()
{
  DebugPrintf("Stop mDNS\n");  
}
