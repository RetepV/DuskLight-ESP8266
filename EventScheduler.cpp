#include <stdlib.h>
#include <algorithm>
#include <time.h>
#include <iostream>
#include <assert.h>
#include <random>

#include "EventScheduler.hpp"

// NOTE: We have to sort on the ACTIVE weekdays and time offsets, as those are the ones that are actually
// used by the scheduler. That's why we have to re-sort every time the active weekdays and time offsets
// might change.
int qsortCompareEventSchedulerItems(const void *a, const void *b) {
    const CEventSchedulerItem* itemA = static_cast<const CEventSchedulerItem*>(a);
    const CEventSchedulerItem* itemB = static_cast<const CEventSchedulerItem*>(b);

    if (itemA->isValid() == false && itemB->isValid() == true) {            // Invalid items go to the end
        return 1;
    } else if (itemA->isValid() == true && itemB->isValid() == false) {     // Invalid items go to the end
        return -1;
    } else if (itemA->activeWeekDay < itemB->activeWeekDay) {                           // Sort by week day
        return -1;
    } else if (itemA->activeWeekDay > itemB->activeWeekDay) {                           // Sort by week day
        return 1;
    } else {
        if (itemA->activeTimeOffset < itemB->activeTimeOffset) {                        // Sort by time offset
            return -1;
        } else if (itemA->activeTimeOffset > itemB->activeTimeOffset) {                 // Sort by time offset
            return 1;
        } else {
            return 0;                                                       // Equal    
        }
    }
}

CEventScheduler::CEventScheduler(double latitude, double longitude, time_t secondsFromGMT, time_t (*timeProvider)(void)) :
sunriseCalculator(CSunriseCalculator(latitude, longitude)),
secondsFromGMT(secondsFromGMT),
timeProvider(timeProvider) {
    randomGenerator.seed(5138008 + timeProvider());
    recalculateAllActivationTimes();
}

CEventScheduler::~CEventScheduler() {
}

void CEventScheduler::resetItems() {
    numberOfStoredItems = 0;
    for (int index = 0; index < numberOfSchedulerItems; index++) {
        items[index] = CEventSchedulerItem();
    }
}

time_t CEventScheduler::getSecondsFromGMT(void) {
    return secondsFromGMT;
}

void CEventScheduler::setSecondsFromGMT(time_t secondsFromGMT) {
    this->secondsFromGMT = secondsFromGMT;
    randomGenerator.seed(5138008 + timeProvider());
    recalculateAllActivationTimes();
}

// NOTE: It is undefined what happens when there are duplicate items (e.g. 2 or more 'sunset' items).
//       In the case of having a random number, the duplication will probably resolve itself fine in
//       most cases. But if there is no randomnes, only the first item is simply returned.
//       Probably it's beneficial to be able to have duplicates. Can we do that without adding
//       something that makes them unique (like an ID)?
int CEventScheduler::addItem(const CEventSchedulerItem item) {

    if (numberOfStoredItems >= numberOfSchedulerItems) {
        return -1; // No space left
    }

    int newItemIndex = numberOfStoredItems;
    
    items[newItemIndex] = item;
    recalculateActivationTime(items[newItemIndex]);

    numberOfStoredItems++;

    sortItems();

    return 0;
}
 
int CEventScheduler::removeItem(const CEventSchedulerItem item) {
    int foundItemIndex = findItemIndex(item);
    if (foundItemIndex >= 0) {
        items[foundItemIndex] = CEventSchedulerItem{}; // Invalidate the item
        sortItems();
        numberOfStoredItems--;
        return 0;
    }
    return -1; // Item not found
}

int CEventScheduler::getNumberOfItems(void) {
    return numberOfStoredItems;
}

CEventSchedulerItem CEventScheduler::getItem(int index) {
    if ((index >= 0) && (index < numberOfStoredItems)) {
        return items[index];

    }
    return CEventSchedulerItem();
}

CEventSchedulerItem CEventScheduler::findItem(const CEventSchedulerItem itemToFind) {
    int foundItemIndex = findItemIndex(itemToFind);
    if (foundItemIndex >= 0) {
        return items[foundItemIndex];
    }

    return CEventSchedulerItem();
}

CEventSchedulerItem CEventScheduler::getActiveItem(void) {

    time_t timestampGMT = timeProvider();
    return getActiveItem(timestampGMT);
}

CEventSchedulerItem CEventScheduler::getNextActiveItem(void) {

    time_t timestampGMT = timeProvider();
    return getNextActiveItem(timestampGMT);
}

CEventSchedulerItem CEventScheduler::getActiveItem(time_t timestampGMT) {

    int index;
    
    if ((index = getActiveItemIndex(timestampGMT)) >= 0) {
        return items[index];
    }

    return CEventSchedulerItem{}; // Return an invalid item
}

CEventSchedulerItem CEventScheduler::getNextActiveItem(time_t timestampGMT) {

    int index;

    if ((index = getActiveItemIndex(timestampGMT)) >= 0) {
        int nextIndex = (index + 1) % numberOfStoredItems;
        return items[nextIndex];
    }

    return CEventSchedulerItem{}; // Return an invalid item
}

int CEventScheduler::getActiveItemIndex(time_t timestampGMT) {

    // If the list is empty, return an invalid item
    if (numberOfStoredItems == 0) {
        // std::cout << "No scheduled events\n";
        return -1;
    }

    // If the list contains only one item, return that item
    if (numberOfStoredItems == 1) {
        // std::cout << "Only one scheduled event\n";
        return 0;
    }

    CEventSchedulerWeekDay weekDay = calculateWeekDay(timestampGMT);
    time_t minutesFromBeginningOfDay = calculateMinutesFromBeginningOfDay(timestampGMT);

    // std::cout << "Current weekDay: " << (int)weekDay << "\n";
    // std::cout << "Current minutes: " << (minutesFromBeginningOfDay / 60) << ":" << std::setfill('0') << std::setw(2) << (minutesFromBeginningOfDay % 60) << "\n";

    // The active item is the last item before or equal to the current time on the current weekday.
    // Easiest way to find it, is to turn that around: iterate backwards through the list of items
    // and look for the first item that is before or equal to the current time on the current weekday.
    // Note that this relies on a sorted list if event items.
    for (int index = numberOfStoredItems - 1; index >= 0; index--) {
        const CEventSchedulerItem& item = items[index];

        // std::cout << "Considering item index: " << index << " - ";
        item.debugPrint();

        if (item.isValid() && item.activeWeekDay <= weekDay) {

            if (item.activeWeekDay == weekDay && item.activeTimeOffset <= minutesFromBeginningOfDay) {
                // If this item has the requested weekday, and its activeTimeoffset is less than the
                // time we have been given, then this is the active item.
                // std::cout << "-> Found. activeWeekDay == weekDay && activeTimeOffset <= minutesFromBeginningOfDay\n";
                return index;   
            }
            else if (item.activeWeekDay < weekDay) {
                // std::cout << "-> Found. activeWeekDay < weekDay\n";
                return index;
            }
        }
    }

    // If we come here, we did not find an active item during iterating. Therefore, IF there is
    //  any active item, it must be the last valid item in the list.
    for (int index = numberOfStoredItems - 1; index >= 0; index--) {
        const CEventSchedulerItem& item = items[index];
        if (item.isValid()) {
            // std::cout << "-> Found. Last item in list\n";
            return index;
        }
    }

    // Basically, we should never come here, because if there are no valid items, we should have returned
    // at the beginning of the function. Assert, but for release builds, just return an error.
    assert(false && "This should never happen, because if there are no valid items, we should have returned at the beginning of the function.");
    
    return -1;
}

int CEventScheduler::findItemIndex(const CEventSchedulerItem itemToFind) {
    for (int i = 0; i < numberOfStoredItems; i++) {
        if (items[i] == itemToFind) {
            return i;
        }
    }
    return -1;
}

int CEventScheduler::calculateBeginningOfWeekInSeconds(time_t timestampGMT) {
    time_t localTime = timestampGMT + secondsFromGMT;

    // std::cout << "  localTime: " << localTime << " (secondsFromGMT: " << secondsFromGMT << ")\n";

    time_t daysSinceEpoch = localTime / secondsInDay;
    time_t secondsIntoDay = localTime % secondsInDay;

    if ((localTime < 0) && (secondsIntoDay != 0)) { daysSinceEpoch--; }

    // std::cout << "  daysSinceEpoch: " << daysSinceEpoch << "\n";
    // std::cout << "  secondsIntoDay: " << secondsIntoDay << "\n";

    CEventSchedulerWeekDay weekDay = calculateWeekDay(timestampGMT);
    time_t startOfWeekDay = daysSinceEpoch - ((time_t)weekDay - 1);
    time_t startOfWeekInLocaltime = startOfWeekDay * secondsInDay;

    // std::cout << "  weekDay: " << weekDay << "\n";
    // std::cout << "  startOfWeekDay: " << startOfWeekDay << "\n";
    // std::cout << "  startOfWeekInLocaltime: " << startOfWeekInLocaltime << "\n";

    // std::cout << "  result: " << startOfWeekInLocaltime - secondsFromGMT << "\n";

    return startOfWeekInLocaltime - secondsFromGMT;
}

int CEventScheduler::calculateBeginningOfDayInSeconds(time_t timestampGMT) {
    time_t localTime = timestampGMT + secondsFromGMT;

    time_t daysSinceEpoch = localTime / secondsInDay;
    time_t secondsIntoDay = localTime % secondsInDay;

    if ((localTime < 0) && (secondsIntoDay != 0)) { daysSinceEpoch--; }

    time_t startOfDayInLocalTime = daysSinceEpoch * secondsInDay;

    return startOfDayInLocalTime - secondsFromGMT;
}

int CEventScheduler::calculateMinutesFromBeginningOfDay(time_t timestampGMT) {
    int minutesIntoDay = calculateSecondsFromBeginningOfDay(timestampGMT) / 60;

    // std::cout << "minutesFromBeginningOfDay: " << minutesIntoDay << " (timestampGMT: " << timestampGMT << ")\n";
    
    return minutesIntoDay;
}

int CEventScheduler::calculateSecondsFromBeginningOfDay(time_t timestampGMT) {
    time_t localTime = timestampGMT + secondsFromGMT;

    time_t secondsIntoDay = localTime % secondsInDay;

    if (secondsIntoDay < 0) { secondsIntoDay += secondsInDay; }

    // std::cout << "secondsFromBeginningOfDay: " << secondsIntoDay << " (timestampGMT: " << timestampGMT << ")\n";

    return secondsIntoDay;
}

CEventSchedulerWeekDay CEventScheduler::calculateWeekDay(time_t timestampGMT) {
    time_t localTime = timestampGMT + secondsFromGMT;

    time_t daysSinceEpoch = localTime / secondsInDay;
    if ((localTime < 0) && ((localTime % secondsInDay) > 0))    { // Timestamp can be negative, fix number of days for that case.
        daysSinceEpoch--;
    }
    time_t weekDay = (time_t)(((time_t)epochWeekDay + daysSinceEpoch) % daysInWeek);
    
    if (weekDay < 0) { weekDay += daysInWeek;}
    if (weekDay == 0) { weekDay = daysInWeek; }

    return static_cast<CEventSchedulerWeekDay>(weekDay);
}

void CEventScheduler::recalculateItemActivationTime(CEventSchedulerItem item) {
    int foundItemIndex = findItemIndex(item);
    if (foundItemIndex >= 0) {
        recalculateActivationTime(items[foundItemIndex]);
        sortItems();
    }
}

time_t CEventScheduler::sunrise(time_t timestampGMT) {
    time_t beginningOfDayInGMT = calculateBeginningOfDayInSeconds(timestampGMT);
    // std::cout << "beginning of day: " << beginningOfDayInGMT << "\n";
    time_t sunriseTime, sunsetTime;

    sunriseCalculator.sunRiseAndSetForTimestamp(beginningOfDayInGMT, 0, sunriseTime, sunsetTime);

    // std::cout << "sunrise/sunset: " << sunriseTime << " - " << sunsetTime << "\n";
    return sunriseTime;
}

time_t CEventScheduler::sunset(time_t timestampGMT) {
    time_t beginningOfDayInGMT = calculateBeginningOfDayInSeconds(timestampGMT);
    time_t sunriseTime, sunsetTime;

    sunriseCalculator.sunRiseAndSetForTimestamp(beginningOfDayInGMT, 0, sunriseTime, sunsetTime);

    return sunsetTime;
}

// Sort all items in ascending order by week day and time offset. Invalid items are moved to the end of the list.
void CEventScheduler::sortItems(void) {
    qsort(items, sizeof(items) / sizeof(items[0]), sizeof(items[0]), qsortCompareEventSchedulerItems);
}

// Update the random offsets of all items. The random offset is used to add some randomness to the activation times
// of the items. This is, for instance, useful when you want to switch a light on at sunset, but don't want to switch
// it on at the exact sunset every day, to make it seem as if there is a person at home switching the light on.
// Same for when to switch the light off.
void CEventScheduler::recalculateAllActivationTimes(void) {
    
    for (int i = 0; i < numberOfStoredItems; i++) {
        if (items[i].isValid()) {
            recalculateActivationTime(items[i]);
        }
    }
    // After recalculating, sort the items again.
    sortItems();
}

void CEventScheduler::recalculateActivationTime(CEventSchedulerItem &item) {

    // std::cout << "recalculateActivationTime item type: " << item.eventType << ", weekDay: " << item.weekDay << ", timeOffset" << item.timeOffset << "\n";

    time_t timestampGMT = timeProvider();

    time_t beginningOfThisWeek = calculateBeginningOfWeekInSeconds(timestampGMT);
    time_t beginningOfDayForItem = beginningOfThisWeek + ((item.weekDay - 1) * secondsInDay);
    time_t sunriseTime, sunsetTime;

    // std::cout << "timestampGMT: " << timestampGMT << "\n";
    // std::cout << "beginningOfThisWeek: " << beginningOfThisWeek << "\n";
    // std::cout << "beginningOfDayForItem: " << beginningOfDayForItem << "\n";

    switch (item.eventType) {
        case CEventSchedulerItemType_Time:
            // For time-based events, we can apply the random offset directly to the time offset.
            // std::cout << "timeOffset is " << item.timeOffset << "\n";
            break;
        case CEventSchedulerItemType_Sunrise:
        case CEventSchedulerItemType_Sunset:
            // For sunrise/sunset-based events, we need to first calculate the sunrise/sunset time
            // for the day of the event and then apply the random offset to that time.
            sunriseCalculator.sunRiseAndSetForTimestamp(beginningOfDayForItem, 0, sunriseTime, sunsetTime);
            // std::cout << "set timeOffset to " << (item.eventType == CEventSchedulerItemType_Sunrise ? "sunrise" : "sunset") << " at " << (item.eventType == CEventSchedulerItemType_Sunrise ? sunriseTime : sunsetTime) << "\n";
            item.timeOffset = calculateMinutesFromBeginningOfDay(item.eventType == CEventSchedulerItemType_Sunrise ? sunriseTime : sunsetTime);
            break;
        default:
            break;
    }

    // std::cout << "randomOffsetMinus is -" << (int)item.randomOffsetMinus << ", randomOffsetPlus is " << (int)item.randomOffsetPlus << "\n";

    std::uniform_int_distribution<int> int_dist(-item.randomOffsetMinus, item.randomOffsetPlus);
    int randomOffset = int_dist(randomGenerator);

    // std::cout << "randomOffset is: " << randomOffset << "\n";

    int16_t newActiveWeekDay = item.weekDay;
    int16_t newActiveTimeOffset = item.timeOffset + randomOffset;

    if (newActiveTimeOffset < 0) {
        item.activeTimeOffset = newActiveTimeOffset + minutesInDay;
        item.activeWeekDay = static_cast<CEventSchedulerWeekDay>((((newActiveWeekDay - 1) - 1 + 7) % 7) + 1); // Move to previous day
    } else if (newActiveTimeOffset >= minutesInDay) {
        item.activeTimeOffset = newActiveTimeOffset - minutesInDay;
        item.activeWeekDay = static_cast<CEventSchedulerWeekDay>((((newActiveWeekDay - 1) + 1) % 7) + 1); // Move to next day
    } else {
        item.activeTimeOffset = newActiveTimeOffset;
        item.activeWeekDay = static_cast<CEventSchedulerWeekDay>(newActiveWeekDay);
    }

    // std::cout << "timeOffset is " << item.timeOffset << "\n";
    // std::cout << "weekDay is " << item.weekDay << "\n";

    // std::cout << "activeTimeOffset is " << item.activeTimeOffset << "\n";
    // std::cout << "activeWeekDay is " << item.activeWeekDay << "\n";
}