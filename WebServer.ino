#include "Globals.h"

// ESP8266Webserver can only handle one simultaneous client, so we can use a global buffer. The size is just a nice number that's big enough. Increase if necessary.
#define WEBPAGE_BUFFER_SIZE 3072
char webPageBuffer[WEBPAGE_BUFFER_SIZE];

#define RESET_PAGE { memset(webPageBuffer, 0, WEBPAGE_BUFFER_SIZE); }
#define ADD_PAGE(STRING) { strcat(webPageBuffer, STRING); }
#define PAGE webPageBuffer

#define META_INFO "<META NAME=\"viewport\" CONTENT=\"width=device-width, initial-scale=1\">"
#define PAGE_STYLE "<STYLE>P{font-family:helvetica;font-size:20}H1{font-family:helvetica}INPUT{font-family:helvetica;font-size:20}TABLE{border:none;border-spacing:0;padding:0}TD{font-family:helvetica;font-size:20;vertical-align:middle;text-align:left;border:none;border-spacing:0;padding:0}</STYLE>"

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
  RESET_PAGE;
  
  ADD_PAGE("<HTML><HEAD>");
  ADD_PAGE(META_INFO);
  ADD_PAGE(PAGE_STYLE);
  ADD_PAGE("<H1>");
  ADD_PAGE(APPLICATION_NAME)
  ADD_PAGE(" ")
  ADD_PAGE(APPLICATION_VERSION)
  ADD_PAGE(" error: page not found</H1></HEAD><BODY><P>")
  ADD_PAGE("<BR><BR>Server: ");
  ADD_PAGE(webServer.uri().c_str())
  ADD_PAGE("<BR>Method: ")
  ADD_PAGE((webServer.method() == HTTP_GET) ? "GET" : "POST")
  ADD_PAGE("<BR>Arguments:<BR>")
  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    ADD_PAGE("&nbsp;")
    ADD_PAGE(webServer.argName(i).c_str())
    ADD_PAGE(":&nbsp;")
    ADD_PAGE(webServer.arg(i).c_str())
    ADD_PAGE("<BR>")
  }
  ADD_PAGE("</P></BODY></HTML>")
  
  webServer.send(404, "text/html", PAGE);
}

void statusPage() 
{
  RESET_PAGE;

  ADD_PAGE("<HTML><HEAD>");
  ADD_PAGE(META_INFO);
  ADD_PAGE(PAGE_STYLE);
  ADD_PAGE("<H1>");
  ADD_PAGE(APPLICATION_NAME)
  ADD_PAGE(" ")
  ADD_PAGE(APPLICATION_VERSION)
  ADD_PAGE("</H1></HEAD><BODY><P>")

  ADD_PAGE("Current NTP time: ")
  if (timeHasBeenSynced)
  {
    ADD_PAGE(NTP.getTimeDateString().c_str())
  }
  else
  {
    ADD_PAGE("Not synced yet")
  }
  
  if (!timeParametersHaveBeenCalculated)
  {
    ADD_PAGE("<BR><BR>Waiting for first calculation of TimeKeeper parameters...")
  }
  else
  {
    if (timeHasBeenSynced)
    {
      ADD_PAGE("<BR><BR>Daylight saving: ")
      ADD_PAGE(isSummerTime() ? " Summer time" : "Winter time")

      char buffer[16] = "";
      
      ADD_PAGE("<BR><BR>Sunrise today: ")
      Dusk2Dawn::min2str(buffer, sunriseTodayMinutes);
      ADD_PAGE(buffer)
      ADD_PAGE("<BR>Sunset today: ")
      Dusk2Dawn::min2str(buffer, sunsetTodayMinutes);
      ADD_PAGE(buffer)
      ADD_PAGE("<BR>Sunrise tomorrow: ")
      Dusk2Dawn::min2str(buffer, sunriseTomorrowMinutes);
      ADD_PAGE(buffer)

      ADD_PAGE("<BR><BR>Switch off today: ")
      Dusk2Dawn::min2str(buffer, switchOffMinutesToday);
      ADD_PAGE(buffer)
      ADD_PAGE("<BR>Switch on today: ")
      Dusk2Dawn::min2str(buffer, switchOnMinutesToday);
      ADD_PAGE(buffer)
      ADD_PAGE("<BR>Switch off tomorrow: ")
      Dusk2Dawn::min2str(buffer, switchOffMinutesTomorrow);
      ADD_PAGE(buffer)
  
      if (settings.timeToSwitchOff < MINUTES_IN_DAY)
      {
        ADD_PAGE("<BR><BR>Switch off at set time: ")
        Dusk2Dawn::min2str(buffer, settings.timeToSwitchOff);
        ADD_PAGE(buffer)
        ADD_PAGE(" (random interval [-")
        sprintf(buffer, "%d", settings.randomMinutesBefore);
        ADD_PAGE(buffer)
        ADD_PAGE(":")
        sprintf(buffer, "%d", settings.randomMinutesAfter);
        ADD_PAGE(buffer)
        ADD_PAGE("]) with current random: ")
        sprintf(buffer, "%d", randomMinutesAdjust);
        ADD_PAGE(buffer)
      }
      else
      {
        ADD_PAGE("<BR><BR>Switching off at dawn")
      }
      
      ADD_PAGE("<BR><BR>Today is ")
      sprintf(buffer, "%d", nowMinutes);
      ADD_PAGE(buffer)
      ADD_PAGE(" minutes old")
  
      int nextEvent = minutesToNextEvent();
      if (nextEvent != 0)
      {
        ADD_PAGE("<BR><BR>Next event is at: ")
        Dusk2Dawn::min2str(buffer, normalizedMinutesIntoDay(minutesIntoDay(now()) + nextEvent));
        ADD_PAGE(buffer)
        ADD_PAGE(" (")
        sprintf(buffer, "%d", nextEvent);
        ADD_PAGE(buffer)
        ADD_PAGE(" minutes)")
      }
    }
  }
  
  ADD_PAGE("<BR><BR><TABLE BORDERWIDTH=\"0\"><TR><TD>Mode is ")
  ADD_PAGE((lightMode() == LightModeAutomatic) ? "auto " : "manual ")
  if (lightMode() == LightModeManual)
  {
    ADD_PAGE("</TD><TD>&nbsp;</TD><FORM ACTION=\"/\" METHOD=\"POST\"><TD><INPUT TYPE=\"HIDDEN\" NAME=\"mode\" VALUE=\"auto\"></INPUT>")
    ADD_PAGE("<INPUT TYPE=\"SUBMIT\" VALUE=\"Return to auto mode\"></TD></FORM></TR>")

    if (shouldResetBackToAuto)
    {
      ADD_PAGE("<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD STYLE=\"font-size:14\">(will reset back to auto at next event)</TD></TR>")
    }
  }
  else
  {
    ADD_PAGE("</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>")
  }
  ADD_PAGE("<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>") 
  ADD_PAGE("<TR><TD>Light is ")
  ADD_PAGE(digitalRead(gpioLight) == 1 ? "off " : "on ")

  ADD_PAGE("</TD><TD>&nbsp;</TD><FORM ACTION=\"/\" METHOD=\"POST\"><TD><INPUT TYPE=\"HIDDEN\" NAME=\"light\" VALUE=\"")
  ADD_PAGE(digitalRead(gpioLight) == 1 ? "on" : "off")
  ADD_PAGE("\"></INPUT>")
  ADD_PAGE("<INPUT TYPE=\"SUBMIT\" VALUE=\"Switch light ")
  ADD_PAGE(digitalRead(gpioLight) == 1 ? "on" : "off")
  ADD_PAGE("\">")
  ADD_PAGE("</TD></FORM></TR></TABLE>")

  ADD_PAGE("</P></BODY></HTML>")

  webServer.send(200, "text/html", PAGE);
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
