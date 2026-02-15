#pragma once

#include <time.h>

class CSunriseCalculator {
public:
    
    CSunriseCalculator(float latitude, float longitude) : latitude(latitude), longitude(longitude) {}

    void sunRiseAndSetForTimestamp(time_t timestampGMT, time_t secondsFromGMT, time_t &sunRise, time_t &sunSet);

private:
    double latitude;
    double longitude;
};
