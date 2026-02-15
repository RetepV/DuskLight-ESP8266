#include "Globals.h"
#include "EventScheduler.hpp"

const double latitude = 52.303713;      // Netherlands
const double longitude = 5.259310;

bool shouldResetBackToAuto = false;
LightMode_t currentLightMode = LightModeAutomatic;

// To actually handle proper TimeZone (or secondsFromGMT) and Daylight Saving Time, we need to use
// the TZInfo database. That contains all the information necessary, with all the exceptions for
// every timezone in the world. Maybe one day I will add that, but for now you will have to
// manually set 
time_t secondsFromGMT = 0;
bool daylightSavingTime = false;

CEventScheduler *scheduler = nullptr;

CEventSchedulerItem currentActiveItem;
CEventSchedulerItem currentNextActiveItem;

void setupTimeKeeper()
{
  // Set up the seconds from GMT and the daylightSavingTime from the settings. The app should not use
  // the settings, but the global variables from TimeKeeper.

  secondsFromGMT = settings.secondsFromGMT;
  daylightSavingTime = settings.daylightSavingTime;

  scheduler = new CEventScheduler(latitude, longitude, secondsFromGMT, [](){ return now(); });

  if (daylightSavingTime) {
    secondsFromGMT += 3600;
  }
}

void programScheduler()
{
  DebugPrintf("Adding events to scheduler\n");

  CEventSchedulerItemType switchOnType = CEventSchedulerItemType_Sunset;
  CEventSchedulerItemType switchOffType = settings.timeToSwitchOff >= MINUTES_IN_DAY ? CEventSchedulerItemType_Sunrise : CEventSchedulerItemType_Time;

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Sunday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Sunday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Monday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Monday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Tuesday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Tuesday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Wednesday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Wednesday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Thursday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Thursday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Friday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Friday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));

  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Saturday, switchOnType, 0, settings.randomMinutesBefore, settings.randomMinutesAfter, 1));
  scheduler->addItem(CEventSchedulerItem(CEventSchedulerDayNumber_Saturday, switchOffType, settings.timeToSwitchOff, settings.randomMinutesBefore, settings.randomMinutesAfter, 0));
}

bool isTimeValid(time_t time)
{
  return time > 946684800;    // Let's assume the time is valid if it's later than January 1, 00:00:00.
}

void setLightMode(LightMode_t lightMode)
{
  DebugPrintf("Set mode to %s\n", lightMode == LightModeManual ? "manual" : "automatic" );
  currentLightMode = lightMode;
  // When we switch to manual, we want to switch back to auto at the next event.
  if (lightMode == LightModeManual)
  {
    shouldResetBackToAuto = true;
  }
}

LightMode_t lightMode()
{
  return currentLightMode;
}

bool lightIsOn()
{
  return digitalRead(gpioLight) == 0;
}

bool lightIsOff()
{
  return !lightIsOn();
}

void switchLight(bool on)
{
  if (on)
  {
    digitalWrite(gpioLed, 0);
    digitalWrite(gpioLight, 0);
  }
  else
  {
    digitalWrite(gpioLed, 1);
    digitalWrite(gpioLight, 1);
  }
}

void handleResetToAuto()
{
  if ((lightMode() == LightModeManual) && shouldResetBackToAuto)
  {
    DebugPrintf("Mode was manual, reset mode back to auto\n");
    setLightMode(LightModeAutomatic);
    shouldResetBackToAuto = false;
  }
}

void handleAutomaticLightState()
{
  DebugPrintf("handleAutomaticLightState\n");
  handleResetToAuto();

  if ((lightMode() == LightModeAutomatic))
  {
    DebugPrintf("Light mode is automatic, switch light %s\n", currentActiveItem.userDefined != 0 ? "ON" : "OFF");
    switchLight(currentActiveItem.userDefined != 0);
  }
}

void checkForEvents()
{
  time_t timestampGMT = now();

  if (isTimeValid(timestampGMT))
  {
    CEventSchedulerItem newActiveItem = scheduler->getActiveItem(timestampGMT);
    CEventSchedulerItem newNextActiveItem = scheduler->getNextActiveItem(timestampGMT);

    if (scheduler->getNumberOfItems() == 0) {
        programScheduler();
    }

    if (!(newActiveItem == currentActiveItem)) {

      currentActiveItem = newActiveItem;
      currentNextActiveItem = newNextActiveItem;

      DebugPrintf("New scheduler event - weekday: %d, minutes into day: %d, userDefined: %d\n", currentActiveItem.activeWeekDay, currentActiveItem.activeTimeOffset, currentActiveItem.userDefined);
      DebugPrintf("Next event - weekday: %d, minutes into day: %d, userDefined: %d\n", currentNextActiveItem.activeWeekDay, currentNextActiveItem.activeTimeOffset, currentNextActiveItem.userDefined);
      
      handleAutomaticLightState();
    }
    else {
      // No change to be made.
    }
  }
  else
  {
    DebugPrintf("Time not valid, keep light off\n");
    // If time is not valid, keep the light off.
    switchLight(false);     
  }
}