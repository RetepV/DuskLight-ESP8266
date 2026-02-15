#include "Globals.h"

// ESP8266Webserver can only handle one simultaneous client, so we can use a global buffer. The size is just
// a nice number that's probably big enough. Increase if necessary.
#define WEBPAGE_BUFFER_SIZE 4096
static char webPageBuffer[WEBPAGE_BUFFER_SIZE];

// Some helper macros
#define RESET_PAGE { memset(webPageBuffer, 0, WEBPAGE_BUFFER_SIZE); }
#define ADD_TO_PAGE(STRING) { strcat(webPageBuffer, STRING); }
#define PAGE webPageBuffer

static char scratchBuffer[16] = "";
#define ADD_MINUTES_TO_PAGE(minutes) { sprintf(scratchBuffer, "%02d:%02d", (int)((int)(minutes) / 60), (int)((int)(minutes) % 60)); ADD_TO_PAGE(scratchBuffer); }
#define ADD_INTEGER_TO_PAGE(integer) { sprintf(scratchBuffer, "%d", (int)(integer)); ADD_TO_PAGE(scratchBuffer); }

#define META_INFO "<meta charset=\"utf-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
#define PAGE_STYLE "<style>\n"\
"body{margin:0;font-family:system-ui,Arial,sans-serif;background-color:#8DA0CB;color:#000}\n"\
".wrap{max-width:980px;margin:0 auto;padding:16px}\n"\
".top{font-size:2rem;font-weight:700;text-align:center;margin:8px 0 18px}\n"\
".topsub{font-size:1rem;font-weight:400;text-align:center;margin:8px 0 18px}\n"\
".smallfont{font-size:0.5rem;font-weight:200;margin:8px 0 18px}\n"\
".grid{\ndisplay:grid;\ngrid-template-columns:repeat(auto-fit,minmax(420px,1fr));\ngap:16px;\nalign-items:flex-start;\nalign-self:flex-start;\n}\n"\
".win{background:#E5C494;border:1px solid #000;box-shadow:10px 10px 0 #404040}\n"\
".title{background:#FC8D62;color:#fff;padding:6px 10px;font-weight:700}\n"\
".content{padding:12px}\n"\
"table{width:100%;border-collapse:collapse}\n"\
"td{padding:4px 0;vertical-align:top}\n"\
"td:last-child{text-align:right}\n"\
"hr{border:0;border-top:1px solid #888;margin:12px 0}\n"\
".row{display:flex;justify-content:space-between;align-items:center;gap:12px;flex-wrap:wrap}\n"\
".btn{display:inline-block;font-size:1rem;padding:6px 10px;background:#d0d0d0;border:1px solid #000;color:#000;text-decoration:none;font-weight:700}\n"\
".footer{text-align:right;font-size:.9rem;margin-top:14px}\n"\
"</style>\n"

#define GRID_START "<div class=\"grid\">"
#define GRID_END "</div>"

#define WINDOW_START(title) "<section class=\"win\">\n<div class=\"title\">" title "</div>\n<div class=\"content\">"
#define WINDOW_END "</div></section>"

ESP8266WebServer webServer(80);

void setupWebserver()
{
  webServer.onNotFound(notFoundPage);

  webServer.on("/", HTTP_GET, statusPage);
  webServer.on("/debug", HTTP_GET, debugPage);
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
  
  ADD_TO_PAGE("<html><head>");
  ADD_TO_PAGE(META_INFO);
  ADD_TO_PAGE(PAGE_STYLE);
  ADD_TO_PAGE("</head><body>")

  ADD_TO_PAGE("<div class=\"top\">")
  ADD_TO_PAGE(APPLICATION_NAME)
  ADD_TO_PAGE(" ")
  ADD_TO_PAGE(APPLICATION_VERSION)
  ADD_TO_PAGE("</div>")
  
  ADD_TO_PAGE("<div class=\"topsub\">")
  ADD_TO_PAGE("error: page not found")
  ADD_TO_PAGE("</div>")

  ADD_TO_PAGE("<div class=\"wrap\">")

  ADD_TO_PAGE(GRID_START);

  ADD_TO_PAGE(WINDOW_START("Info"))

  ADD_TO_PAGE("<table>")
  ADD_TO_PAGE("<tr><td><strong>URL</strong></td><td>");
  ADD_TO_PAGE(webServer.uri().c_str())
  ADD_TO_PAGE("</td></tr>")
  ADD_TO_PAGE("<tr><td><strong>Method</strong></td><td>");
  ADD_TO_PAGE((webServer.method() == HTTP_GET) ? "GET" : "POST");
  ADD_TO_PAGE("</td></tr>")
  ADD_TO_PAGE("<tr><td><strong>Arguments</strong></td><td>");
  for (uint8_t i = 0; i < webServer.args(); i++) {
    ADD_TO_PAGE("<p>")
    ADD_TO_PAGE(webServer.argName(i).c_str())
    ADD_TO_PAGE(":&nbsp;")
    ADD_TO_PAGE(webServer.arg(i).c_str())
    ADD_TO_PAGE("</p>")
  } 
  ADD_TO_PAGE("</td></tr>")
  ADD_TO_PAGE("</table>")
  ADD_TO_PAGE(WINDOW_END)

  ADD_TO_PAGE("<></body></html>")
  
  webServer.send(404, "text/html", PAGE);
}

void statusPage() 
{
  RESET_PAGE;

  ADD_TO_PAGE("<html><head>")
  ADD_TO_PAGE(META_INFO)
  ADD_TO_PAGE(PAGE_STYLE)
  ADD_TO_PAGE("</head><body>")

  // Wrap and top

  ADD_TO_PAGE("<table>")
  ADD_TO_PAGE("<tr><td><div class=\"topsub\">Seconds from GMT: ");
  ADD_INTEGER_TO_PAGE(secondsFromGMT);
  ADD_TO_PAGE("</div></td><td>")
  ADD_TO_PAGE("<div class=\"top\">")
  time_t currentGMTTimestamp = now();
  time_t currentLocalTimestamp = currentGMTTimestamp + secondsFromGMT;
  if (timeHasBeenSynced)
  {
    ADD_TO_PAGE(asctime(gmtime(&currentLocalTimestamp)))
  }
  else
  {
    ADD_TO_PAGE("Waiting for time to become valid...")
  }

  ADD_TO_PAGE("</div></td><td><div class=\"topsub\">Daylight saving: ")
  ADD_TO_PAGE(daylightSavingTime ? "Summer time" : "Winter time");
  ADD_TO_PAGE("</div></td></tr>")
  ADD_TO_PAGE("</table>")  
  
  ADD_TO_PAGE("<br><br>")

  ADD_TO_PAGE("<div class=\"wrap\">")

  if (isTimeValid(now()))
  {
    // Grid
    ADD_TO_PAGE(GRID_START);

    ADD_TO_PAGE(WINDOW_START("Sunrise and sunset"))

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Sunrise today</strong></td><td>");
    time_t sunriseToday = scheduler->sunrise(currentLocalTimestamp) + secondsFromGMT;
    ADD_TO_PAGE(asctime(gmtime(&sunriseToday)));
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("<tr><td><strong>Sunset today</strong></td><td>");
    time_t sunsetToday = scheduler->sunset(currentLocalTimestamp) + secondsFromGMT;
    ADD_TO_PAGE(asctime(gmtime(&sunsetToday)));
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("<tr><td><strong>Sunrise tomorrow</strong></td><td>");
    time_t sunriseTomorrow = scheduler->sunrise(currentLocalTimestamp + 86400) + secondsFromGMT;
    ADD_TO_PAGE(asctime(gmtime(&sunriseTomorrow)));
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE(WINDOW_END) // Sunrise and sunset

    ADD_TO_PAGE(WINDOW_START("Scheduler"))

    CEventSchedulerItem activeItem = scheduler->getActiveItem(currentGMTTimestamp);
    CEventSchedulerItem nextActiveItem = scheduler->getNextActiveItem(currentGMTTimestamp);
    time_t beginningOfDay = scheduler->calculateBeginningOfDayInSeconds(currentGMTTimestamp);

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Minutes into current day</strong></td><td>")
    ADD_MINUTES_TO_PAGE(scheduler->calculateMinutesFromBeginningOfDay(currentGMTTimestamp));
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE("<hr>")

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Currently active item started at</strong></td><td>")
    ADD_MINUTES_TO_PAGE(activeItem.activeTimeOffset);
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("<tr><td><strong>Next active item starts at</strong></td><td>")
    ADD_MINUTES_TO_PAGE(nextActiveItem.activeTimeOffset);
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE("<hr>")

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Switching on at</strong></td><td>")
    ADD_TO_PAGE("Sunset")
    ADD_TO_PAGE("</td></tr>")

    ADD_TO_PAGE("<tr><td><strong>Switching off at</strong></td><td>")
    if (settings.timeToSwitchOff < MINUTES_IN_DAY) {
      ADD_MINUTES_TO_PAGE(settings.timeToSwitchOff)
    }
    else {
      ADD_TO_PAGE("Sunrise")
    }
    ADD_TO_PAGE("</td></tr>")

    ADD_TO_PAGE("<tr><td><strong>Random interval (minutes)</strong></td><td>")
    ADD_TO_PAGE("[-")
    sprintf(scratchBuffer, "%d", settings.randomMinutesBefore);
    ADD_TO_PAGE(scratchBuffer)
    ADD_TO_PAGE(":")
    sprintf(scratchBuffer, "%d", settings.randomMinutesAfter);
    ADD_TO_PAGE(scratchBuffer)
    ADD_TO_PAGE("]")
    ADD_TO_PAGE("</td></tr>")

    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE(WINDOW_END)   // Scheduler

    ADD_TO_PAGE(WINDOW_START("Mode"))

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Mode</strong></td><td>")
    ADD_TO_PAGE((lightMode() == LightModeAutomatic) ? "auto " : "manual ")
    if ((lightMode() == LightModeManual) && shouldResetBackToAuto) {
      ADD_TO_PAGE("<div class=\"smallfont\">(will reset back to auto at next event)</div>")
    }
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE("<hr>")

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Light is ")
    ADD_TO_PAGE(digitalRead(gpioLight) == 1 ? "OFF" : "ON")
    ADD_TO_PAGE("</strong></td><td>")

    if (lightMode() == LightModeManual) {
      ADD_TO_PAGE("<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"mode\" value=\"auto\"></input>")
      ADD_TO_PAGE("<input class=\"btn\" type=\"submit\" value=\"Return to auto mode\"></form>")
    }

    ADD_TO_PAGE("<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"light\" value=\"")
    ADD_TO_PAGE(digitalRead(gpioLight) == 1 ? "on" : "off")
    ADD_TO_PAGE("\"></input>")
    ADD_TO_PAGE("<input class=\"btn\" type=\"submit\" value=\"Switch light ")
    ADD_TO_PAGE(digitalRead(gpioLight) == 1 ? "on" : "off")
    ADD_TO_PAGE("\"></form>")

    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE(WINDOW_END)   // Mode

    ADD_TO_PAGE(GRID_END);
  }
 
  ADD_TO_PAGE("<div class=\"footer\">")
  ADD_TO_PAGE(APPLICATION_NAME)
  ADD_TO_PAGE("&nbsp;")
  ADD_TO_PAGE(APPLICATION_VERSION)
  ADD_TO_PAGE("&nbsp;&nbsp;&nbsp;")
  uint32_t freeHeapSize = ESP.getFreeHeap();
  uint32_t largestHeapBlockSize = ESP.getMaxFreeBlockSize();
  ADD_INTEGER_TO_PAGE(freeHeapSize)
  ADD_TO_PAGE(" - ")
  ADD_INTEGER_TO_PAGE(largestHeapBlockSize)
  ADD_TO_PAGE("</div>")
 
  ADD_TO_PAGE("</div>")  // wrap
  
  ADD_TO_PAGE("</body></html>")

  webServer.send(200, "text/html", PAGE);
}

void postPage()
{
  if (webServer.arg("mode") == "auto")
  {
    // Switch to automatic mode.
    DebugPrintf("Light mode automatic\n");
    setLightMode(LightModeAutomatic);
    handleAutomaticLightState();
  }
  else
  {
    // Manual override light on or off.
    if (webServer.arg("light") == "on")
    {
      DebugPrintf("Light mode manual on\n");
      setLightMode(LightModeManual);
      switchLight(true);
    }
    else if (webServer.arg("light") == "off") 
    {
      DebugPrintf("Light mode manual off\n");
      setLightMode(LightModeManual);
      switchLight(false);
    }
  }

  webServer.sendHeader("location", "/");
  webServer.send(302, "text/plain", "");
}

void debugPage() {
   RESET_PAGE;
  
  ADD_TO_PAGE("<html><head>");
  ADD_TO_PAGE(META_INFO);
  ADD_TO_PAGE(PAGE_STYLE);
  ADD_TO_PAGE("</head><body>")

  ADD_TO_PAGE("<table>")
  ADD_TO_PAGE("<tr><td><div class=\"topsub\">Seconds from GMT: ");
  ADD_INTEGER_TO_PAGE(secondsFromGMT);
  ADD_TO_PAGE("</div></td><td>")
  ADD_TO_PAGE("<div class=\"top\">")
  time_t currentGMTTimestamp = now();
  time_t currentLocalTimestamp = currentGMTTimestamp + secondsFromGMT;
  if (timeHasBeenSynced)
  {
    ADD_TO_PAGE(asctime(gmtime(&currentLocalTimestamp)))
  }
  else
  {
    ADD_TO_PAGE("Waiting for time to become valid...")
  }

  ADD_TO_PAGE("</div></td><td><div class=\"topsub\">Daylight saving: ")
  ADD_TO_PAGE(daylightSavingTime ? "Summer time" : "Winter time");
  ADD_TO_PAGE("</div></td></tr>")
  ADD_TO_PAGE("</table>")  

  ADD_TO_PAGE("<div class=\"topsub\">")
  ADD_TO_PAGE(APPLICATION_NAME)
  ADD_TO_PAGE(" ")
  ADD_TO_PAGE(APPLICATION_VERSION)
  ADD_TO_PAGE("</div>")
  
  ADD_TO_PAGE("<div class=\"topsub\">")
  ADD_TO_PAGE("Debug info")
  ADD_TO_PAGE("</div>")

  ADD_TO_PAGE("<div class=\"wrap\">")

  ADD_TO_PAGE(GRID_START);

  if (scheduler != nullptr) {

    ADD_TO_PAGE(WINDOW_START("Schedule"))

    ADD_TO_PAGE("<table>")
    for (int index = 0; index < scheduler->getNumberOfItems(); index++) {
      CEventSchedulerItem item = scheduler->getItem(index);
      ADD_TO_PAGE("<tr>")
      ADD_TO_PAGE("<td>")
      ADD_TO_PAGE(item.evenTypeAsString())
      ADD_TO_PAGE("</td><td>")
      ADD_TO_PAGE(item.weekDayAsString())
      ADD_TO_PAGE("</td><td>")
      ADD_MINUTES_TO_PAGE(item.timeOffset)
      ADD_TO_PAGE(" [-")
      ADD_INTEGER_TO_PAGE(item.randomOffsetMinus)
      ADD_TO_PAGE(",")
      ADD_INTEGER_TO_PAGE(item.randomOffsetPlus)
      ADD_TO_PAGE("]")
      ADD_TO_PAGE("</td><td>")
      ADD_TO_PAGE(item.activeWeekDayAsString())
      ADD_TO_PAGE("</td><td>")
      ADD_MINUTES_TO_PAGE(item.activeTimeOffset)
      ADD_TO_PAGE("</td><td>")
      ADD_INTEGER_TO_PAGE(item.userDefined)
      ADD_TO_PAGE("</td>")
      ADD_TO_PAGE("</tr>")
    }

    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE(WINDOW_END)

    ADD_TO_PAGE(WINDOW_START("Scheduler"))

    CEventSchedulerItem activeItem = scheduler->getActiveItem(currentGMTTimestamp);
    CEventSchedulerItem nextActiveItem = scheduler->getNextActiveItem(currentGMTTimestamp);
    time_t beginningOfDay = scheduler->calculateBeginningOfDayInSeconds(currentGMTTimestamp);

    ADD_TO_PAGE("<table>")

    time_t minutesIntoDay = scheduler->calculateMinutesFromBeginningOfDay(currentGMTTimestamp);
    CEventSchedulerItem tempItem = CEventSchedulerItem(scheduler->calculateWeekDay(currentGMTTimestamp), CEventSchedulerItemType_Time, minutesIntoDay, 0, 0, 0);
    ADD_TO_PAGE("<tr><td><strong>Minutes into current day</strong></td><td>")
    ADD_TO_PAGE(tempItem.weekDayAsString())
    ADD_TO_PAGE("</td><td>")
    ADD_MINUTES_TO_PAGE(minutesIntoDay);
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE("<hr>")

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Currently active item started at</strong></td><td>")
    ADD_TO_PAGE(activeItem.weekDayAsString())
    ADD_TO_PAGE("</td><td>")
    ADD_MINUTES_TO_PAGE(activeItem.activeTimeOffset);
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("<tr><td><strong>Next active item starts at</strong></td><td>")
    ADD_TO_PAGE(nextActiveItem.weekDayAsString())
    ADD_TO_PAGE("</td><td>")
    ADD_MINUTES_TO_PAGE(nextActiveItem.activeTimeOffset);
    ADD_TO_PAGE("</td></tr>")
    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE("<hr>")

    ADD_TO_PAGE("<table>")
    ADD_TO_PAGE("<tr><td><strong>Switching on at</strong></td><td>")
    ADD_TO_PAGE("Sunset")
    ADD_TO_PAGE("</td></tr>")

    ADD_TO_PAGE("<tr><td><strong>Switching off at</strong></td><td>")
    if (settings.timeToSwitchOff < MINUTES_IN_DAY) {
      ADD_MINUTES_TO_PAGE(settings.timeToSwitchOff)
    }
    else {
      ADD_TO_PAGE("Sunrise")
    }
    ADD_TO_PAGE("</td></tr>")

    ADD_TO_PAGE("<tr><td><strong>Random interval (minutes)</strong></td><td>")
    ADD_TO_PAGE("[-")
    sprintf(scratchBuffer, "%d", settings.randomMinutesBefore);
    ADD_TO_PAGE(scratchBuffer)
    ADD_TO_PAGE(":")
    sprintf(scratchBuffer, "%d", settings.randomMinutesAfter);
    ADD_TO_PAGE(scratchBuffer)
    ADD_TO_PAGE("]")
    ADD_TO_PAGE("</td></tr>")

    ADD_TO_PAGE("</table>")

    ADD_TO_PAGE(WINDOW_END)   // Scheduler
  }

  ADD_TO_PAGE(GRID_END);

  ADD_TO_PAGE("<div class=\"footer\">")
  ADD_TO_PAGE(APPLICATION_NAME)
  ADD_TO_PAGE("&nbsp;")
  ADD_TO_PAGE(APPLICATION_VERSION)
  ADD_TO_PAGE("&nbsp;&nbsp;&nbsp;")
  uint32_t freeHeapSize = ESP.getFreeHeap();
  uint32_t largestHeapBlockSize = ESP.getMaxFreeBlockSize();
  ADD_INTEGER_TO_PAGE(freeHeapSize)
  ADD_TO_PAGE(" - ")
  ADD_INTEGER_TO_PAGE(largestHeapBlockSize)
  ADD_TO_PAGE("</div>")

  ADD_TO_PAGE("</body></html>")
  
  webServer.send(200, "text/html", PAGE);
}
