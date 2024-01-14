#include "Globals.h"

ESP8266WebServer webServer(80);

void setupWebserver()
{
  webServer.onNotFound(notFoundPage);

  webServer.on("/", HTTP_GET, statusPage);
  webServer.on("/", HTTP_POST, postPage);
}

void startWebserver() 
{
  webServer.begin();
}

void stopWebserver()
{
  webServer.stop();
}

// Webserver pages

void notFoundPage()
{
  String message = "";

  message += "<HTML><HEAD><FONT FACE=\"Helvetica\"><H1>DuskLight error: page not found</H1></FONT></HEAD><BODY>";
  message += "<FONT FACE=\"Helvetica\">URI: ";
  message += webServer.uri();
  message += "<BR>Method: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "<BR>Arguments: ";
  message += webServer.args();
  message += "<BR>";
  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    message += "&nbsp;" + webServer.argName(i) + ":&nbsp;" + webServer.arg(i) + "<BR>";
  }
  message += "</FONT></BODY></HTML>";

  webServer.send(404, "text/html", message);
}

void statusPage() 
{
  String message = "";

  message += "<HTML><HEAD><FONT FACE=\"Helvetica\"><H1>DuskLight V1.3.1</H1></FONT></HEAD><BODY>";
  
  message += "<FONT FACE=\"Helvetica\">Current NTP time: ";
  message += timeHasBeenSynced ? NTP.getTimeDateString() : "Not synced yet";

  if (!hasLastRecalculated)
  {
    message += "<BR><BR>Waiting for first calculation of TimeKeeper parameters...";
  }
  else
  {
    if (timeHasBeenSynced)
    {
      message += "<BR><BR>Daylight saving: ";
      message += isSummerTime() ? " Summer time" : "Winter time";
      
      message += "<BR><BR>Sunrise today: ";
      message += timeStringForMinutesIntoDay(sunriseTodayMinutes);
      message += "<BR>Sunset today: ";
      message += timeStringForMinutesIntoDay(sunsetTodayMinutes);
      message += "<BR>Sunrise tomorrow: ";
      message += timeStringForMinutesIntoDay(sunriseTomorrowMinutes);
  
      message += "<BR><BR>Switch off today: ";
      message += timeStringForMinutesIntoDay(switchOffMinutesToday);
      message += "<BR>Switch on today: ";
      message += timeStringForMinutesIntoDay(switchOnMinutesToday);
      message += "<BR>Switch off tomorrow: ";
      message += timeStringForMinutesIntoDay(switchOffMinutesTomorrow);
  
      if (settings.timeToSwitchOff < MINUTES_IN_DAY)
      {
        message += "<BR><BR>Switch off at set time: ";
        message += timeStringForMinutesIntoDay(settings.timeToSwitchOff);
        message += " (random interval [-";
        message += String(settings.randomMinutesBefore);
        message += ":";
        message += String(settings.randomMinutesAfter);
        message += "]) with current random: ";
        message += String(randomMinutesAdjust);
      }
      else
      {
        message += "<BR><BR>Switching off at dawn";
      }
      
      message += "<BR><BR>Today is ";
      message += nowMinutes;
      message += " minutes old";
  
      int nextEvent = minutesToNextEvent();
      if (nextEvent != 0)
      {
        message += "<BR><BR>Next event is at: ";
        message += timeStringForMinutesIntoDay(normalizedMinutesIntoDay(minutesIntoDay(now()) + nextEvent));
        message += " (";
        message += String(nextEvent);
        message += " minutes)";
      }
    }
  }

  message += "<BR><BR><TABLE BORDERWIDTH=\"0\"><TR><TD STYLE=\"vertical-align:middle;\">Mode is ";
  message += (lightMode() == LightModeAutomatic) ? "auto " : "manual ";
  if (lightMode() == LightModeManual)
  {
    message += "</TD><FORM ACTION=\"/\" METHOD=\"POST\"><TD STYLE=\"vertical-align:middle;\"><INPUT TYPE=\"HIDDEN\" NAME=\"mode\" VALUE=\"auto\"></INPUT>";
    message += "<INPUT TYPE=\"SUBMIT\" VALUE=\"Return to auto mode\"></TD></FORM></TR>";

    if (shouldResetBackToAuto)
    {
      message += "<TR><TD>&nbsp;</TD><TD>";
      message += "Resets to auto at ";
      message += NTP.getTimeDateString(timeToResetBackToAuto);
      message += "</TD></TR>";
    }
  }
  else
  {
    message += "</TD></TR>";
  }
    
  message += "<TR><TD STYLE=\"vertical-align:middle;\">Light is ";
  message += digitalRead(gpioLight) == 1 ? "off " : "on ";

  message += "</TD><FORM ACTION=\"/\" METHOD=\"POST\"><TD STYLE=\"vertical-align:middle;\"><INPUT TYPE=\"HIDDEN\" NAME=\"light\" VALUE=\"";
  message += digitalRead(gpioLight) == 1 ? "on" : "off";
  message += "\"></INPUT>";
  message += "<INPUT TYPE=\"SUBMIT\" VALUE=\"Switch light ";
  message += digitalRead(gpioLight) == 1 ? "on" : "off";
  message += "\">";
  message += "</TD></FORM></TR></TABLE>";

  message += "</FONT></BODY></HTML>";

  webServer.send(200, "text/html", message);
}

void postPage()
{
  if (webServer.arg("mode") == "auto")
  {
    // Automatic mode.
    DebugPrintf("Light mode automatic\n");
    setLightMode(LightModeAutomatic);
    checkForLightActions();
  }
  else
  {
    // Manual override light on or off.
    if (webServer.arg("light") == "on")
    {
      DebugPrintf("Light mode manual on\n");
      setLightMode(LightModeManual);
      switchLightOn();
    }
    else if (webServer.arg("light") == "off") 
    {
      DebugPrintf("Light mode manual off\n");
      setLightMode(LightModeManual);
      switchLightOff();
    }
  }

  webServer.sendHeader("location", "/");
  webServer.send(302, "text/plain", "");
}
