#include "Globals.h"

Dusk2Dawn huizen(52.298, 5.234, 1);

bool timeParametersHaveBeenCalculated = false;

bool shouldResetBackToAuto = false;

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

bool isTimeValid(time_t time)
{
  return time > 946684800;
}

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

void timeStringForMinutesIntoDay(int minutesIntoDay, char *timeString)
{
  Dusk2Dawn::min2str(timeString, minutesIntoDay);
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

  time_t currentTime = now();

  if (!isTimeValid(currentTime))
  {
    DebugPrintf("Have time %lld, but it's not valid\n", currentTime);
    return;
  }
  
  summerTime = NTP.isSummerTimePeriod(currentTime);

  nowMinutes = minutesIntoDay(currentTime);

  // Note: This 946684800 is 1-1-2000 and is used as a crude check to check if the time is valid.
  if ((nowMinutes == 0) || (randomMinutesAdjust == -1))
  {
    // Recalculate new random minutes. Let's recalculate once per day at 00:00.
    updateRandomMinutesAdjust();
  }

  TimeElements timeElementsNow;
  breakTime(currentTime, timeElementsNow);

  sunriseTodayMinutes = huizen.sunrise(timeElementsNow.Year, timeElementsNow.Month, timeElementsNow.Day, summerTime);
  sunsetTodayMinutes = huizen.sunset(timeElementsNow.Year, timeElementsNow.Month, timeElementsNow.Day, summerTime);

  TimeElements timeElementsTomorrow;
  breakTime(currentTime + SECONDS_IN_DAY, timeElementsTomorrow);

  sunriseTomorrowMinutes = huizen.sunrise(timeElementsTomorrow.Year, timeElementsTomorrow.Month, timeElementsTomorrow.Day, summerTime);

  switchOffMinutesToday = sunriseTodayMinutes;
  switchOnMinutesToday = sunsetTodayMinutes;
  switchOffMinutesTomorrow = sunriseTomorrowMinutes;
    
  if (settings.timeToSwitchOff <= MINUTES_IN_DAY)
  {
    switchOffMinutesToday = settings.timeToSwitchOff + randomMinutesAdjust;
    switchOffMinutesTomorrow = switchOffMinutesToday;
  }

  timeParametersHaveBeenCalculated = true;  

#ifdef DEBUG
  char timeString[] = "00:00";
  DebugPrintf("Daylight saving    : %s time\n", summerTime ? "Summer" : "Winter");
  Dusk2Dawn::min2str(timeString, sunriseTodayMinutes);
  DebugPrintf("Sunrise today      : %s\n", timeString);
  Dusk2Dawn::min2str(timeString, sunsetTodayMinutes);
  DebugPrintf("Sunset today       : %s\n", timeString);
  Dusk2Dawn::min2str(timeString, sunriseTomorrowMinutes);
  DebugPrintf("Sunrise tomorrow   : %s\n", timeString);
  Dusk2Dawn::min2str(timeString, switchOffMinutesToday);
  DebugPrintf("Switch off today   : %s\n", timeString);
  Dusk2Dawn::min2str(timeString, switchOnMinutesToday);
  DebugPrintf("Switch on today    : %s\n", timeString);
  Dusk2Dawn::min2str(timeString, switchOffMinutesTomorrow);
  DebugPrintf("Switch off tomorrow: %s\n", timeString);  
#endif
}

int minutesToNextEvent()
{
  int nextEvent = -1;

  if (timeParametersHaveBeenCalculated)
  {
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
    char timeString[] = "00:00";
    Dusk2Dawn::min2str(timeString, normalizedMinutesIntoDay(minutesIntoDay(now()) + nextEvent));
    DebugPrintf("Next event is in %d minutes (%s)\n", nextEvent, timeString);
#endif
  }
  else
  {
    DebugPrintf("TimeKeeper parameters not recalculated yet, nextEvent is unknown");
  }

  return nextEvent;
}

void handleLightState()
{
  if (timeParametersHaveBeenCalculated)
  {

    // Check if lightmode should be reset to auto, and reset if necessary.
  
    if ((lightMode() == LightModeManual) && (minutesToNextEvent() == 0 ) && shouldResetBackToAuto)
    {
      DebugPrintf("Reset mode back to auto\n");
      setLightMode(LightModeAutomatic);
      shouldResetBackToAuto = false;
    }

    // Check if we should switch the light on or off now.

    if ((lightMode() == LightModeAutomatic))
    {
      if (lightIsOff() && lightShouldBeOn())
      {
        DebugPrintf("Automatic light switch on\n");
        switchLightOn();      
      }
      else if (lightIsOn() && lightShouldBeOff())
      {
        DebugPrintf("Automatic light switch off\n");
        switchLightOff();
      }
    }
  }
  else
  {
    switchLightOff();     
  }
}

void checkForLightActions()
{
  if (timeHasBeenSynced)
  {
    recalculateTimeKeeperParameters();
    DebugPrintf("Current time is %s (%s time), mode is %s, it's %s, light is %s, light should be %s.\n",
                  NTP.getTimeDateString().c_str(),
                  isSummerTime() ? "summer" : "winter",
                  (lightMode() == LightModeAutomatic) ? "auto" : "manual",
                  itsDark() ? "dark" : "light",
                  lightIsOn() ? "on" : "off",
                  lightShouldBeOn() ? "on" : "off");
    handleLightState();
  }
  else
  {
    DebugPrintf("Current time is not synced yet, time parameters have not been calculated yet.\n");
  }
}

void setLightMode(LightMode_t lightMode)
{
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

bool isSummerTime()
{
  return summerTime;
}

bool itsDark()
{
  return (nowMinutes < sunriseTodayMinutes) || (nowMinutes >= sunsetTodayMinutes);
}

bool lightIsOn()
{
  return digitalRead(gpioLight) == 0;
}

bool lightIsOff()
{
  return !lightIsOn();
}

bool lightShouldBeOn()
{
  return (nowMinutes < switchOffMinutesToday) || (nowMinutes >= switchOnMinutesToday);
}

bool lightShouldBeOff()
{
  return (nowMinutes >= switchOffMinutesToday) && (nowMinutes < switchOnMinutesToday);
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
