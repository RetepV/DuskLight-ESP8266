#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Defines and macros

// #define DEBUG
#define RELEASE

#define APPLICATION_NAME "DuskLight"
#define APPLICATION_VERSION "1.3.4"

#define AP_NAME "DuskLightAP"
#define AP_PASSWORD "DuskLight"

#define DEFAULT_MDNS_HOSTNAME "dusklight"
// Sadly we need a const string for this and can't compose.
#define DEFAULT_MDNS_PLACEHOLDER_STRING "placeholder=\"dusklight\""

#define MINUTES_IN_DAY (24 * 60)
#define SECONDS_IN_DAY (MINUTES_IN_DAY * 60)

// Used 
#define VALID_TIME_SECONDS 946684800

#define HOSTNAME_MAXLENGTH 40

#ifdef DEBUG
  #define DEBUG_NTPCLIENT 1

  #define DebugPrintf(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_NTPCLIENT 0

  #define DebugPrintf(...) ((void)0)
#endif

#define EXECUTE_PERIODICALLY(TIMEVAR, MILLISECONDS, CODE) {\
  static int TIMEVAR = 0;\
  if ((millis() - TIMEVAR) > (MILLISECONDS)) {\
    DebugPrintf("EXECUTE_PERIODICALLY " #TIMEVAR "\n");\
    TIMEVAR = millis();\
    CODE\
  }\
}

// Includes

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include <NtpClientLib.h>
#include <TimeLib.h>
#include <time.h>

#include <stdio.h>

#include <math.h>
#include <Dusk2Dawn.h>

#include <EEPROM.h>

// Global types

typedef enum LightMode
{
  LightModeAutomatic = 0,
  LightModeManual
} LightMode_t;

typedef enum ScheduleEntryType
{
  Time = 0,
  Dawn,
  Dusk
} ScheduleEntryType_t;

typedef struct SettingsType {
  unsigned char signature[2] = { 0x55, 0xAA };
  unsigned char version = 0x00;
  char hostname[HOSTNAME_MAXLENGTH + 1] = DEFAULT_MDNS_HOSTNAME;
  int16_t timeToSwitchOff = MINUTES_IN_DAY + 1; // If larger than MINUTES_IN_DAY, then the time to switch off is considered to not have been set.
  uint8_t randomMinutesBefore = 0;
  uint8_t randomMinutesAfter = 0;
} SettingsType_t;

// GLobal constants

const int gpioLed = 2;
const int gpioLight = 4;
const int gpioButton = 5;

// Global variable prototypes

extern int previousButtonState;

extern int sunriseTodayMinutes;
extern int sunsetTodayMinutes;
extern int sunriseTomorrowMinutes;

extern int switchOffMinutesToday;
extern int switchOnMinutesToday;
extern int switchOffMinutesTomorrow;

extern int nowMinutes;

extern int randomMinutesAdjust;

extern bool timeParametersHaveBeenCalculated;

// Function prototypes and other forward declarations

// TimeKeeper

time_t getBeginningOfDay(time_t timeOfDay);
int minutesIntoDay(time_t timeOfDay);
int normalizedMinutesIntoDay(int minutes);
int minutesToNextEvent();

void checkForLightActions();

// Settings

extern SettingsType_t settings;

void setupSettings();
void startSettings();
void stopSettings();
String settingsDebugString();

// MDNS

void setupMDNS();
void startMDNS();
void stopMDNS();

// NTP

extern boolean shouldProcessNTPEvent;
extern boolean timeHasBeenSynced;

void setupNTP();
void startNTP();
void stopNTP();

// WiFi

extern boolean wifiConnected;

void setupWiFi();
void startWiFiServices();
void stopWiFiServices();
void resetWiFiSettings();

void setupServices();
void startServices();
void stopServices();

// Webserver

extern ESP8266WebServer webServer;

void setupWebserver();
void startWebserver();
void stopWebserver();

#endif // _GLOBALS_H_
