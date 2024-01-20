#include "Globals.h"

Dusk2Dawn huizen(52.298, 5.234, 1);

bool hasLastRecalculated = false;
time_t timeLastRecalculated;

bool shouldResetBackToAuto = false;
time_t timeToResetBackToAuto;

bool summerTime = false;

int sunriseTodayMinutes;
int sunsetTodayMinutes;
int sunriseTomorrowMinutes;

int switchOffMinutesToday;
int switchOnMinutesToday;
int switchOffMinutesTomorrow;

int nowMinutes;

int randomMinutesAdjust = -1;

LightMode_t currentLightMode = LightModeAutomatic;

// Time functions

time_t getBeginningOfDay(time_t timeOfDay)
{
  TimeElements tmBeginningOfDay;
  breakTime(timeOfDay, tmBeginningOfDay);

  tmBeginningOfDay.Second = 0;
  tmBeginningOfDay.Minute = 0;
  tmBeginningOfDay.Hour = 0;

  return makeTime(tmBeginningOfDay);
}

int minutesIntoDay(time_t timeOfDay)
{
  time_t beginningOfDay = getBeginningOfDay(timeOfDay);
  int secondsToday = timeOfDay - beginningOfDay;
  return (secondsToday / 60);
}

int normalizedMinutesIntoDay(int minutes)
{
  while (minutes >= MINUTES_IN_DAY)
  {
    minutes -= MINUTES_IN_DAY;
  }
  return minutes;
}

String timeStringForMinutesIntoDay(int minutesIntoDay)
{
  char timeString[] = "00:00";
  Dusk2Dawn::min2str(timeString, minutesIntoDay);
  return String(timeString);
}

int minutesToNextEvent()
{
  int nextEvent = -1;

  if (hasLastRecalculated == true)
  {
    DebugPrintf("Recalculating minutesToNextEvent.\n");
    
    switchOffMinutesToday = sunriseTodayMinutes;
    switchOnMinutesToday = sunsetTodayMinutes;
    switchOffMinutesTomorrow = sunriseTomorrowMinutes;
    
    if (settings.timeToSwitchOff <= MINUTES_IN_DAY)
    {
      switchOffMinutesToday = settings.timeToSwitchOff + randomMinutesAdjust;
      switchOffMinutesTomorrow = switchOffMinutesToday;
    }

    if (nowMinutes <= switchOffMinutesToday)
    {
      // It's before sunrise today, first dark part of the day => next event is at sunrise today.
      nextEvent = switchOffMinutesToday - nowMinutes;
    }
    else if (nowMinutes <= switchOnMinutesToday)
    {
      // It's before sunset today, light part of the day => next event is sunset today.
      nextEvent = switchOnMinutesToday - nowMinutes;
    }
    else
    {
      // It's after sunset today, second dark part of the day => next event is (slightly after) sunrise tomorrow.
      nextEvent = ((24 * 60) - nowMinutes) + switchOffMinutesTomorrow;
    }

    #ifdef DEBUG
    DebugPrintf("Daylight saving    : %s time\n", summerTime ? "Summer" : "Winter");
    DebugPrintf("Switch off today   : %s\n", timeStringForMinutesIntoDay(switchOffMinutesToday).c_str());
    DebugPrintf("Switch on today    : %s\n", timeStringForMinutesIntoDay(switchOnMinutesToday).c_str());
    DebugPrintf("Switch off tomorrow: %s\n", timeStringForMinutesIntoDay(switchOffMinutesTomorrow).c_str());  
    DebugPrintf("Next event in      : %d minutes\n", nextEvent);
    DebugPrintf("Next event at      : %s\n", timeStringForMinutesIntoDay(normalizedMinutesIntoDay(minutesIntoDay(now()) + nextEvent)).c_str());  
    #endif
  }
  else
  {
    DebugPrintf("TimeKeeper parameters not recalculated yet, nextEvent is unknown");
  }

  return nextEvent;
}

void updateRandomMinutesAdjust()
{
  int randMod = settings.randomMinutesBefore + settings.randomMinutesAfter;
  
  if (randMod > 0)
  {
    DebugPrintf("Seed random with time: %lld\n", now());
    srand(now());
    int randMinutes = rand() % randMod;
    DebugPrintf("Updated random minutes - minsBefore:%d minsAfter:%d randMod:%d -> randMinutes:%d \n",
                   settings.randomMinutesBefore, settings.randomMinutesAfter, randMod, randMinutes);
    randomMinutesAdjust = randMinutes - settings.randomMinutesBefore;
  }
  else
  {
    randomMinutesAdjust = 0;
  }

  DebugPrintf("New random minutes adjust: %d\n", randomMinutesAdjust);
}

void recalculateTimeKeeperParameters()
{
  DebugPrintf("Recalculate TimeKeeper parameters\n");
  hasLastRecalculated = true;
  timeLastRecalculated = now();

  summerTime = NTP.isSummerTimePeriod(timeLastRecalculated);

  nowMinutes = minutesIntoDay(timeLastRecalculated);

  DebugPrintf("now(): %lld\n", now());

  // Note: 946684800 is 1-1-2000. The time is for sure valid when it's more than 1-1-2000.
  if ((now() > 946684800) && ((nowMinutes == 0) || (randomMinutesAdjust == -1)))
  {
    // Recalculate new random minutes. Let's recalculate once per day at 00:00.
    updateRandomMinutesAdjust();
  }

  TimeElements timeElementsNow;
  breakTime(timeLastRecalculated, timeElementsNow);

  sunriseTodayMinutes = huizen.sunrise(timeElementsNow.Year, timeElementsNow.Month, timeElementsNow.Day, summerTime);
  sunsetTodayMinutes = huizen.sunset(timeElementsNow.Year, timeElementsNow.Month, timeElementsNow.Day, summerTime);

  TimeElements timeElementsTomorrow;
  breakTime(timeLastRecalculated + (24 * 60 * 60), timeElementsTomorrow);

  sunriseTomorrowMinutes = huizen.sunrise(timeElementsTomorrow.Year, timeElementsTomorrow.Month, timeElementsTomorrow.Day, summerTime);

#ifdef DEBUG
  char timeToNextEventStr[] = "00:00";
  Dusk2Dawn::min2str(timeToNextEventStr, minutesToNextEvent());
  DebugPrintf("Time to next event : %s\n", timeToNextEventStr);
#endif
}

void handleLightState()
{
  // Check if lightmode should be reset, and reset if necessary.
  
  if ((lightMode() == LightModeManual) && shouldResetBackToAuto)
  {
    if (now() > timeToResetBackToAuto)
    {
      setLightMode(LightModeAutomatic);
      timeToResetBackToAuto = 0;
      shouldResetBackToAuto = false;
    }
  }

  // Check if we should switch the light on or off.

  if ((lightMode() == LightModeAutomatic) && timeHasBeenSynced)
  {
    if (itsDark() && lightIsOff())
    {
      DebugPrintf("Automatic light switch on\n");
      switchLightOn();      
    }
    else if (!itsDark() && lightIsOn())
    {
      DebugPrintf("Automatic light switch off\n");
      switchLightOff();
    }
  }  
}

void checkForLightActions()
{
  if (timeHasBeenSynced)
  {
    recalculateTimeKeeperParameters();
    DebugPrintf("- Time is %s (%s time), it's %s, light is %s, mode is %s.\n",
                  NTP.getTimeDateString().c_str(),
                  isSummerTime() ? "summer" : "winter",
                  itsDark() ? "dark" : "light",
                  lightIsOn() ? "on" : "off",
                  (lightMode() == LightModeAutomatic) ? "auto" : "manual");
    handleLightState();
  }
  else
  {
    DebugPrintf("- Time is not synced yet\n");
  }
}

void setLightMode(LightMode_t lightMode)
{
  currentLightMode = lightMode;
  
  // When we switch to manual, always set up an automatic reset of lightMode at the next event.
  if (lightMode == LightModeManual)
  {
    timeToResetBackToAuto = getBeginningOfDay(now()) + ((nowMinutes + minutesToNextEvent()) * 60);
    shouldResetBackToAuto = true;
  }
}

LightMode_t lightMode()
{
  return currentLightMode;
}

bool isSummerTime()
{
  return summerTime;
}

bool itsDark()
{
  return ((nowMinutes < sunriseTodayMinutes) || (nowMinutes > sunsetTodayMinutes));
}

bool lightIsOn()
{
  return digitalRead(gpioLight) == 0;
}

bool lightIsOff()
{
  return !lightIsOn();
}

void switchLightOn()
{
  digitalWrite(gpioLed, 0);
  digitalWrite(gpioLight, 0);
}

void switchLightOff()
{
  digitalWrite(gpioLed, 1);
  digitalWrite(gpioLight, 1);
}
