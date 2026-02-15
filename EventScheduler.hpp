#pragma once

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <random>

#include "EventSchedulerItem.hpp"
#include "SunriseCalculator.hpp"

// NOTES
//
// The items store the events by weekday and number of minutes into that day. This is useful for three reasons: it uses less memory,
// is timezone independent and is easier to represent in a user interface.
//
// However, we also need to work with the actual time, which is a time_t timestamp. And weekdays are timezone-dependent. So there
// is a choice of working with a timestamp in GMT and a timezone (or offset in seconds from GMT), or work with localtime, which
// already contains the offset in seconds.
// In both cases, the scheduler will still need to know the offset in seconds from GMT, for making certain corrections. So the
// choice that was made for this class is to pass the seconds from timezone in the constructor (or through setSecondsFromGMT()),
// and have all the timestamps in and out of this class be in GMT. This means that all time_t values are considered as being
// in GMT time. The caller application can then add secondsFromGMT to convert the timestamps into local time for UI purposes.
//
// For example consider the following function:
//
//     CEventSchedulerWeekDay calculateWeekDay(time_t timestampGMT);
//
// This expects a timestamp in GMT, but WILL calculate the proper week day considering the secondsFromGMT.
//
//     time_t sunrise(time_t timestampGMT);
//
// Same here, we pass the GMT timestamp of the day, and the returned timestamp will be in GMT too. Add the seconds from GMT to
// get local time.

class CEventScheduler {
private:
    typedef std::mt19937    RandomMT19937Generator;

    static const int        numberOfSchedulerItems = 70;        // We hold storage for on average 10 events per day, 70 events per week max.

    static const int        minutesInDay = 60 * 24;
    static const int        secondsInDay = 60 * minutesInDay;
    static const int        daysInWeek = 7;
    static const CEventSchedulerWeekDay epochWeekDay = CEventSchedulerDayNumber_Thursday;

    CEventSchedulerItem     items[numberOfSchedulerItems];
    int                     numberOfStoredItems = 0;

    CSunriseCalculator      sunriseCalculator;
    RandomMT19937Generator  randomGenerator;

    time_t                  secondsFromGMT;
    time_t                  (*timeProvider)(void);

public:
    CEventScheduler(double latitude, double longitude, time_t secondsFromGMT, time_t (*timeProvider)(void));
    ~CEventScheduler();

    time_t getSecondsFromGMT(void);
    void setSecondsFromGMT(time_t secondsFromGMT);

    void resetItems(void);

    int addItem(const CEventSchedulerItem item);
    int removeItem(const CEventSchedulerItem item);

    int getNumberOfItems(void);

    CEventSchedulerItem getItem(int index);
    CEventSchedulerItem findItem(const CEventSchedulerItem itemToFind);

    CEventSchedulerItem getActiveItem(void);
    CEventSchedulerItem getNextActiveItem(void);

    CEventSchedulerItem getActiveItem(time_t timestampGMT);
    CEventSchedulerItem getNextActiveItem(time_t timestampGMT);

    time_t sunrise(time_t timestampGMT);
    time_t sunset(time_t timestampGMT);

    int calculateMinutesFromBeginningOfDay(time_t timestampGMT);
    int calculateSecondsFromBeginningOfDay(time_t timestampGMT);
    int calculateBeginningOfDayInSeconds(time_t timestampGMT);
    int calculateBeginningOfWeekInSeconds(time_t timestampGMT);
    CEventSchedulerWeekDay calculateWeekDay(time_t timestampGMT);

private:

    int getActiveItemIndex(time_t timestampGMT);
    int findItemIndex(const CEventSchedulerItem itemToFind);

    void recalculateAllActivationTimes(void);
    void recalculateActivationTime(CEventSchedulerItem &item);
    void sortItems(void);
};