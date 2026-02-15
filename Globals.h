#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Defines and macros

#define DEBUG
// #define RELEASE

#define APPLICATION_NAME "DuskLight"
#define APPLICATION_VERSION "1.4.1"

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

#include <EEPROM.h>

// Global types

typedef enum LightMode
{
  LightModeAutomatic = 0,
  LightModeManual
} LightMode_t;

#define SETTINGS_VERSION      0x00

typedef struct SettingsType {
  unsigned char signature[2] = { 0x55, 0xAA };
  unsigned char version = SETTINGS_VERSION;
  char hostname[HOSTNAME_MAXLENGTH + 1] = DEFAULT_MDNS_HOSTNAME;
  int16_t timeToSwitchOff = MINUTES_IN_DAY + 1; // If larger than MINUTES_IN_DAY, then the time to switch off is considered to not have been set.
  uint8_t randomMinutesBefore = 0;
  uint8_t randomMinutesAfter = 0;
  int32_t secondsFromGMT = 0;
  bool    daylightSavingTime = false;
} SettingsType_t;

// GLobal constants

const int gpioLed = 2;
const int gpioLight = 4;
const int gpioButton = 5;

// Global variable prototypes

extern int previousButtonState;

extern bool timeIsValid;

// Function prototypes and other forward declarations

// TimeKeeper

extern time_t secondsFromGMT;

void setupTimeKeeper();
void handleAutomaticLightState();
void checkForEvents();

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

extern boolean wifiDidConnectEvent;

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
