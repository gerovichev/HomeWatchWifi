#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include <Arduino.h>
#include "birthdays.h"

class CalendarManager {
public:
    // Constructor
    CalendarManager();

    // Methods to interact with calendar data
    void readCalendarEvents();              // Reads events from birthdays.h
    void printNextEventToScreen() const;    // Prints next upcoming event
    bool shouldUpdateToday() const;         // Checks if calendar should be updated today
    void markUpdated();                     // Marks calendar as updated for today

private:
    // Member variables to store calendar event data
    String nextEventTitle;
    String nextEventTime;
    time_t nextEventStartTime;
    time_t nextEventEndTime;
    bool hasEvent;
    int lastUpdateDay;  // Last day when calendar was updated (1-31)
    unsigned long lastDisplayTime;  // Last time event was displayed (millis)
    
    // Helper methods
    String formatEventTime(time_t eventTime) const;
    String formatEventTimeRange(time_t startTime, time_t endTime) const;
    String truncateEventTitle(const String& title, int maxLength = 20) const;
    void findNextEvent();                   // Find next upcoming event
    bool shouldDisplayNow() const;         // Check if event should be displayed now (once per 15 min)
    bool isEventActiveNow() const;         // Check if current event is active (within time range)
    bool matchesHostname(const char* eventHostname) const; // Check if event matches current board hostname
};

#endif // CALENDAR_MANAGER_H
