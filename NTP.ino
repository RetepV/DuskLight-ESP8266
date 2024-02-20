
// Constants

const int8_t    ntpTimeZone = 1;
// const String    ntpTimeServer = "time.euro.apple.com";
const String    ntpTimeServer = "es.pool.ntp.org";
const int8_t    ntpSyncRate = 63;

// Variables

NTPSyncEvent_t  ntpEvent; // Last NTP event that was triggered

boolean         timeHasBeenSynced = false;
boolean         shouldProcessNTPEvent = false;

void setupNTP()
{
  DebugPrintf("Setup NTP\n");
  
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    DebugPrintf("NTP sync event detected\n");
    ntpEvent = event;
    shouldProcessNTPEvent = true;
  });

  timeHasBeenSynced = false;
}

void startNTP()
{
  DebugPrintf("Start NTP\n");
  NTP.begin(ntpTimeServer, ntpTimeZone, true);
  NTP.setInterval(ntpSyncRate);
  NTP.getFirstSync();
}

void stopNTP()
{
  DebugPrintf("Stop NTP\n");
  NTP.stop();
}

void processNTPSyncEvent()
{
  DebugPrintf("Handle received NTP sync event: ");
  
  switch (ntpEvent)
  {
    case timeSyncd:
      DebugPrintf("NTP time synchronized: %s\n", NTP.getTimeDateString(NTP.getLastNTPSync()).c_str());
      setTime(NTP.getLastNTPSync());
      timeHasBeenSynced = true;
      break;
    case requestSent:
      DebugPrintf("NTP request sent\n");
      break;
    case noResponse:
      DebugPrintf("ERROR - No response, server not reachable\n");
      // If time has not been synced yet and server is not reachable, retry immediately.
      if (!timeHasBeenSynced)
      {
        stopNTP();
        startNTP();
      }
      break;
    case invalidAddress:
      DebugPrintf("ERROR - Invalid NTP server address\n");
      break;
    case errorSending:
      DebugPrintf("ERROR - Failed to send NTP request\n");
      break;
    case responseError:
      DebugPrintf("ERROR - Received unexpected response\n");
      break;
    default:
      DebugPrintf("ERROR - Received unknown NTP event:%d\n", ntpEvent);
      break;
  }
}
