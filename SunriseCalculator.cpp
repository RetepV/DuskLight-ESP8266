#include <ctime>
#include <cmath>
#include <iostream>

#include "SunriseCalculator.hpp"

// https://en.wikipedia.org/wiki/Sunrise_equation

#define UNIX_TO_JULIAN(unixTime) (((double)(unixTime) / 86400.0) + 2440587.5)
#define JULIAN_TO_UNIX(julianDate) (((double)(julianDate) - 2440587.5) * 86400.0)

#define DEG_TO_RAD(x) (((double)x) * M_PI / 180.0)
#define RAD_TO_DEG(x) (((double)x) * 180.0 / M_PI)

void CSunriseCalculator::sunRiseAndSetForTimestamp(time_t timestampGMT, time_t secondsFromGMT, time_t &sunRise, time_t &sunSet) {
    // Calculate Julian day of year
    double julianDayOfYear = ceil(UNIX_TO_JULIAN(timestampGMT + secondsFromGMT) - (2451545.0 + 0.0009) + (69.184 / 86400.0));
    // Mean solar time
    double meanSolarTime = julianDayOfYear + 0.0009 - (longitude / 360.0);
    // Solar mean anomaly
    double solarMeanAnomaly = std::fmod(357.5291 + 0.98560028 * meanSolarTime, 360.0);
    if (solarMeanAnomaly < 0) solarMeanAnomaly += 360.0; // Necessary?
    // Equation of the center
    double equationOfTheCenter = (1.9148 * std::sin(DEG_TO_RAD(solarMeanAnomaly)) + 
                                 0.02 * std::sin(2 * DEG_TO_RAD(solarMeanAnomaly)) + 
                                 0.0003 * std::sin(3 * DEG_TO_RAD(solarMeanAnomaly)));
    // Ecliptic longitude
    double eclipticLongitude = std::fmod(solarMeanAnomaly + equationOfTheCenter + 180.0 + 102.9372, 360.0);
    if (eclipticLongitude < 0) eclipticLongitude += 360.0;  // Necessary?
    // Solar transit
    double solarTransit = 2451545.0 + 
                         meanSolarTime +
                         0.0053 * std::sin(DEG_TO_RAD(solarMeanAnomaly)) - 
                         0.0069 * std::sin(2 * DEG_TO_RAD(eclipticLongitude));
    time_t solarTransitTime = static_cast<std::time_t>(JULIAN_TO_UNIX(solarTransit));
    // Declination of the sun
    double sunDecline = RAD_TO_DEG(std::asin(std::sin(DEG_TO_RAD(eclipticLongitude)) * 
                                  std::sin(DEG_TO_RAD(23.4393))));
    // Hour angle
    double hourAngle = RAD_TO_DEG(std::acos((std::sin(DEG_TO_RAD(-0.833)) - 
                                 std::sin(DEG_TO_RAD(latitude)) * std::sin(DEG_TO_RAD(sunDecline))) / 
                                (std::cos(DEG_TO_RAD(latitude)) * std::cos(DEG_TO_RAD(sunDecline)))));
    // Sunrise and sunset in Julian Day Number
    double sunriseJulianDay = solarTransit - hourAngle / 360.0;
    double sunsetJulianDay = solarTransit + hourAngle / 360.0;
    // Calculate back to unix time
    sunRise = static_cast<std::time_t>(JULIAN_TO_UNIX(sunriseJulianDay));
    sunSet = static_cast<std::time_t>(JULIAN_TO_UNIX(sunsetJulianDay));
}
