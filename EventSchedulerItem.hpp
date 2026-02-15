#include "TimeLib.h"
#pragma once

#include <stdio.h>
#include <stdint.h>

enum CEventSchedulerWeekDay: uint8_t {
    CEventSchedulerDayNumber_Uninitialized = 0,
    CEventSchedulerDayNumber_Sunday,
    CEventSchedulerDayNumber_Monday,
    CEventSchedulerDayNumber_Tuesday,
    CEventSchedulerDayNumber_Wednesday,
    CEventSchedulerDayNumber_Thursday,
    CEventSchedulerDayNumber_Friday,
    CEventSchedulerDayNumber_Saturday
};

enum CEventSchedulerItemType: uint8_t {
    CEventSchedulerItemType_Spare = 0,                  // Unused, you can use it if you can come up with something useful
    CEventSchedulerItemType_Time,                       // Specific time of day       
    CEventSchedulerItemType_Sunrise,                    // Sunrise 
    CEventSchedulerItemType_Sunset                      // Sunset
};

// #include "DebugStuff.hpp"

struct CEventSchedulerItem {

    CEventSchedulerWeekDay weekDay:3;
    // 3 bits
    uint16_t timeOffset:11; // Time offset in minutes from the beginning of the day, e.g. 0 for 00:00, 60 for 1:00, 1439 for 23:59.
    // 14 bits
    uint8_t randomOffsetMinus:7; // Random time from 0..127 minutes before the timeOffset.
    // 21 bits
    uint8_t randomOffsetPlus:7; // Random time from 0..127 minutes after the timeOffset.
    // 28 bits
    CEventSchedulerItemType eventType:2;    // Type of the event, e.g. time, sunrise, sunset, etc.
    // 30 bits
    CEventSchedulerWeekDay activeWeekDay:3; // Due to the random time offset, the event may become active on a different week day.
    // 33 bits
    uint16_t activeTimeOffset:11;           // Due to the random time offset, the event may become active at a different time.
    // 44 bits
    uint8_t userDefined:4;      // User defined data, for example, to store an ID or something else.
    // 48 bits

    CEventSchedulerItem() :
        weekDay(CEventSchedulerDayNumber_Uninitialized),
        timeOffset(0),
        randomOffsetMinus(0),
        randomOffsetPlus(0),
        eventType(CEventSchedulerItemType_Spare),
        activeWeekDay(CEventSchedulerDayNumber_Uninitialized),
        activeTimeOffset(0),
        userDefined(0)
    {}

    CEventSchedulerItem(
        CEventSchedulerWeekDay weekDay,
        CEventSchedulerItemType eventType,
        unsigned int timeOffset,
        int randomOffsetMinus,
        int randomOffsetPlus,
        uint8_t userDefined
    ) :
        weekDay(weekDay),
        timeOffset(timeOffset),
        randomOffsetMinus(randomOffsetMinus),
        randomOffsetPlus(randomOffsetPlus),
        eventType(eventType),
        activeWeekDay(CEventSchedulerDayNumber_Uninitialized),
        activeTimeOffset(0),
        userDefined(userDefined)
    {}

    bool isValid() const {
        return weekDay != CEventSchedulerDayNumber_Uninitialized;
    }

    bool operator==(const CEventSchedulerItem &other) const {
        if (eventType == other.eventType && weekDay == other.weekDay) {
            if (eventType == CEventSchedulerItemType_Sunrise ||
                eventType == CEventSchedulerItemType_Sunset) {
                if (userDefined == other.userDefined) {
                    return true;
                }
            }
            else if (timeOffset == other.timeOffset &&
                     userDefined == other.userDefined) {
                return true;
            }
        }
        return false;
    }

    const char *evenTypeAsString() {
        switch (eventType) {
            case CEventSchedulerItemType_Spare: return "Spare";
            case CEventSchedulerItemType_Time: return "Time";
            case CEventSchedulerItemType_Sunrise: return "Sunrise";
            case CEventSchedulerItemType_Sunset: return "Sunset";
            default: return "Invalid";
        }
    }

    const char *weekDayAsString() {
        return _weekDayAsString(weekDay);
    }

    const char *activeWeekDayAsString() {
        return _weekDayAsString(activeWeekDay);
    }

    void debugPrint() const {
        std::cout << "CEventSchedulerItem: type " << eventType
                  << " on day " << weekDay << " at " << (timeOffset / 60) << ":" << std::setfill('0') << std::setw(2) << (timeOffset % 60)
                  << " with rnd [-" << (int)randomOffsetMinus << "," << (int)randomOffsetPlus << "]"
                  << ", activates on day " << activeWeekDay << " at " << (activeTimeOffset / 60) << ":" << std::setfill('0') << std::setw(2) << (activeTimeOffset % 60)
                  << " - value: " << (int)userDefined
                  << "\n";
    }

private:
    const char *_weekDayAsString(CEventSchedulerWeekDay weekday) {
        switch (weekday) {
            case CEventSchedulerDayNumber_Uninitialized: return "Uninitialized";
            case CEventSchedulerDayNumber_Sunday: return "Sunday";
            case CEventSchedulerDayNumber_Monday: return "Monday";
            case CEventSchedulerDayNumber_Tuesday: return "Tuesday";
            case CEventSchedulerDayNumber_Wednesday: return "Wednesday";
            case CEventSchedulerDayNumber_Thursday: return "Thursday";
            case CEventSchedulerDayNumber_Friday: return "Friday";
            case CEventSchedulerDayNumber_Saturday: return "Saturday";
            default: return "Invalid";
        }
    }
};