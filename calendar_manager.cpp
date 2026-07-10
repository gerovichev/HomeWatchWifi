#include "calendar_manager.h"
#include "logger.h"
#include "TimeManager.h"
#include "global_config.h"
#include "led_display.h"
#include "device_state.h"
#include <TimeLib.h>
#include "clock.h"

CalendarManager::CalendarManager() {
    // Initialize calendar data members
    nextEventTitle = "";
    nextEventTime = "";
    nextEventStartTime = 0;
    nextEventEndTime = 0;
    hasEvent = false;
    lastUpdateDay = -1;  // -1 means never updated
    lastDisplayTime = 0;  // Never displayed
}

// Check if event matches current board hostname
bool CalendarManager::matchesHostname(const char* eventHostname) const {
    if (eventHostname == nullptr || strlen(eventHostname) == 0) {
        return true;
    }
    // Compare directly without wrapping in a temporary String
    return (DeviceState::getInstance().getHostname() == eventHostname);
}

// Find next upcoming event
void CalendarManager::findNextEvent() {
    hasEvent = false;
    nextEventTitle = "";
    nextEventTime = "";
    nextEventStartTime = 0;
    nextEventEndTime = 0;
    
    if (calendarEventsCount == 0) {
        return;
    }
    
    time_t now = timeClient.getEpochTime();
    if (now < 946684800) {
        return; // Time not synchronized
    }
    
    // Use gmtime_r to avoid shared static buffer
    struct tm timeinfoNow;
    if (gmtime_r(&now, &timeinfoNow) == nullptr) {
        return;
    }

    int currentMonth = timeinfoNow.tm_mon + 1;
    int currentDay   = timeinfoNow.tm_mday;
    int currentHour  = timeinfoNow.tm_hour;
    int currentYear  = timeinfoNow.tm_year + 1900;

    const BirthdayEvent* nextEvent = nullptr;
    time_t nextEventStartTimeValue = 0;
    time_t nextEventEndTimeValue = 0;
    int minDaysAhead = 366; // More than a year
    
    // Get current board hostname for priority checking — stored once, not rebuilt per iteration
    String currentHostname = DeviceState::getInstance().getHostname();

    // Find the next event within next 365 days
    for (int i = 0; i < calendarEventsCount; i++) {
        const BirthdayEvent* event = &calendarEvents[i];

        if (!matchesHostname(event->hostname)) {
            continue;
        }

        if (event->hostname == nullptr || strlen(event->hostname) == 0) {
            bool hasSpecificEventForThisDate = false;
            for (int j = 0; j < calendarEventsCount; j++) {
                const BirthdayEvent* checkEvent = &calendarEvents[j];
                if (checkEvent->hostname != nullptr &&
                    strlen(checkEvent->hostname) > 0 &&
                    currentHostname == checkEvent->hostname &&    // no String() wrap
                    checkEvent->month == event->month &&
                    checkEvent->day == event->day) {
                    hasSpecificEventForThisDate = true;
                    break;
                }
            }
            if (hasSpecificEventForThisDate) {
                continue;
            }
        }

        // Calculate days until this event
        int daysUntilEvent = 0;
        bool isToday = (event->month == currentMonth && event->day == currentDay);
        bool isPastToday = false;
        
        // Check if event is today but already passed
        if (isToday) {
            if (event->fromHour >= 0) {
                // Timed event - check if it already passed
                if (event->toHour >= 0) {
                    // Event has end time - check if current time is after end time
                    if (currentHour > event->toHour) {
                        isPastToday = true;
                    }
                } else {
                    // Event has start time but no end time (shouldn't happen, but handle it)
                    // Consider it passed if current hour is after start hour + 1
                    if (currentHour > event->fromHour + 1) {
                        isPastToday = true;
                    }
                }
            } else {
                // All-day event - check if it's past midnight (next day)
                // All-day events are valid until 23:59, so they're never past today
                isPastToday = false;
            }
        }
        
        // Calculate event start time
        struct tm eventStartTm = {0};
        if (event->month > currentMonth || 
            (event->month == currentMonth && event->day > currentDay) ||
            (isToday && !isPastToday)) {
            // Event is this year (today or future)
            eventStartTm.tm_year = currentYear - 1900;
        } else {
            // Event is next year
            eventStartTm.tm_year = currentYear - 1900 + 1;
        }
        eventStartTm.tm_mon = event->month - 1;
        eventStartTm.tm_mday = event->day;
        eventStartTm.tm_hour = (event->fromHour >= 0) ? event->fromHour : 0;
        eventStartTm.tm_min = 0;
        eventStartTm.tm_sec = 0;
        
        time_t eventStartTime = mktime(&eventStartTm);
        
        // Calculate event end time
        struct tm eventEndTm = eventStartTm;
        if (event->toHour >= 0) {
            eventEndTm.tm_hour = event->toHour;
            eventEndTm.tm_min = 59;
        } else {
            // All-day event ends at 23:59
            eventEndTm.tm_hour = 23;
            eventEndTm.tm_min = 59;
        }
        time_t eventEndTime = mktime(&eventEndTm);
        
        // Skip if event already ended today
        if (isToday && isPastToday) {
            continue;
        }
        
        // Calculate days until event
        daysUntilEvent = (eventStartTime - now) / 86400;
        
        // Check if this is the closest upcoming event
        if (daysUntilEvent >= 0 && daysUntilEvent < minDaysAhead) {
            minDaysAhead = daysUntilEvent;
            nextEvent = event;
            nextEventStartTimeValue = eventStartTime;
            nextEventEndTimeValue = eventEndTime;
        }
    }
    
    if (nextEvent != nullptr && minDaysAhead <= 365) {
        hasEvent = true;

        if (nextEvent->boardTitle != nullptr && strlen(nextEvent->boardTitle) > 0) {
            nextEventTitle = nextEvent->boardTitle;   // direct const char* → String assign
        } else {
            nextEventTitle = nextEvent->title;
        }

        nextEventStartTime = nextEventStartTimeValue;
        nextEventEndTime   = nextEventEndTimeValue;

        if (nextEvent->fromHour >= 0) {
            if (nextEvent->toHour >= 0 && nextEvent->toHour != nextEvent->fromHour) {
                nextEventTime = formatEventTimeRange(nextEventStartTime, nextEventEndTime);
            } else {
                nextEventTime = formatEventTime(nextEventStartTime);
            }
        } else {
            nextEventTime = "All day";   // no temporary String() wrapper
        }

        LOG_INFO_FMT("Next event found: %s on %d-%d%s (in %d days)",
                     nextEventTitle.c_str(),
                     nextEvent->month, nextEvent->day,
                     nextEvent->fromHour >= 0
                         ? (" at " + nextEventTime).c_str()
                         : " (all day)",
                     minDaysAhead);
    }
}

// Function to read calendar events from birthdays.h
void CalendarManager::readCalendarEvents() {
    LOG_INFO_F("Reading calendar events from birthdays.h...");

    findNextEvent();
    markUpdated();

    if (hasEvent) {
        LOG_INFO_FMT("Calendar event loaded: %s", nextEventTitle.c_str());
    } else {
        LOG_INFO_F("No upcoming events found");
    }
}

// Check if event should be displayed now (once per 15 minutes)
bool CalendarManager::shouldDisplayNow() {
    unsigned long currentTime = millis();
    const unsigned long DISPLAY_INTERVAL_MS = 15 * 60 * 1000; // 15 minutes in milliseconds
    
    // If never displayed or 15 minutes passed, display it
    if (lastDisplayTime == 0 || (currentTime - lastDisplayTime >= DISPLAY_INTERVAL_MS)) {
        return true;
    }
    
    return false;
}

// Check if current event is active (within time range)
bool CalendarManager::isEventActiveNow() const {
    if (!hasEvent) {
        return false;
    }
    
    time_t now = timeClient.getEpochTime();
    return (now >= nextEventStartTime && now <= nextEventEndTime);
}

// Function to print next event on the screen
void CalendarManager::printNextEventToScreen() {
    if (!hasEvent || nextEventTitle.length() == 0) {
        LOG_DEBUG_F("No events to present, skipping stage");
        Clock::getInstance().skipCurrentDisplay();
        return;
    }

    // Check if event is today - only show events on the day of the event
    time_t now = timeClient.getEpochTime();
    // Bug fix #1: gmtime() returns a shared static buffer; use gmtime_r() with
    // separate stack structs so the two calls don't overwrite each other.
    struct tm timeinfo_now, timeinfo_event;
    if (gmtime_r(&now, &timeinfo_now) == nullptr) {
        Clock::getInstance().skipCurrentDisplay();
        return;
    }
    
    int currentMonth = timeinfo_now.tm_mon + 1;
    int currentDay   = timeinfo_now.tm_mday;
    
    // Get event date from stored start time
    if (gmtime_r(&nextEventStartTime, &timeinfo_event) == nullptr) {
        Clock::getInstance().skipCurrentDisplay();
        return;
    }
    
    int eventMonth = timeinfo_event.tm_mon + 1;
    int eventDay   = timeinfo_event.tm_mday;
    
    bool isEventToday = (eventMonth == currentMonth && eventDay == currentDay);
    
    // Only show events that are today
    if (!isEventToday) {
        LOG_DEBUG_FMT("Event is not today (current: %d/%d, event: %d/%d), skipping stage",
                      currentMonth, currentDay, eventMonth, eventDay);
        Clock::getInstance().skipCurrentDisplay();
        return;
    }
    
    // Event is today - check if it's currently active (for timed events)
    bool isActive = isEventActiveNow();
    
    LOG_DEBUG_FMT("Event is today, isActive=%s", isActive ? "true" : "false");

    // For timed events, only show when active. For all-day events, show all day
    // Check if we should display now (once per 15 minutes)
    // But always display if event is currently active (or if it's an all-day event)
    if (!shouldDisplayNow() && !isActive) {
        // Skip display
        Clock::getInstance().skipCurrentDisplay();
        return;
    }

    // Format: "Time Title" or "Title" for all-day events
    String tape;
    
    if (isActive) {
        // Event is active now - show title prominently
        tape = truncateEventTitle(nextEventTitle, 20);
    } else if (nextEventTime.length() > 0 && nextEventTime != "All day") {
        // Show time and truncated title
        // Extract first time from range if it's a range
        String shortTime = nextEventTime;
        int spacePos = shortTime.indexOf(' ');
        if (spacePos > 0) {
            shortTime = shortTime.substring(0, spacePos); // Take first part (start time)
        }
        if (shortTime.length() > 5) {
            shortTime = shortTime.substring(0, 5); // Limit to HH:MM
        }
        String shortTitle = truncateEventTitle(nextEventTitle, 15);
        tape = shortTime + " " + shortTitle;
    } else {
        // Show only title
        tape = truncateEventTitle(nextEventTitle, 20);
    }
    
    LOG_INFO_FMT(">> Display: Calendar Event = %s%s",
                 tape.c_str(), isActive ? " (ACTIVE)" : "");
    drawString(tape);

    lastDisplayTime = millis();
}

// Helper: Format event time as HH:MM — use gmtime_r for re-entrant safety
String CalendarManager::formatEventTime(time_t eventTime) const {
    struct tm tmValue;
    if (gmtime_r(&eventTime, &tmValue) == nullptr) {
        return String("--:--");
    }
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", tmValue.tm_hour, tmValue.tm_min);
    return String(timeStr);
}

// Helper: Format event time range as "HH:MM-HH:MM"
String CalendarManager::formatEventTimeRange(time_t startTime, time_t endTime) const {
    // Bug fix #9: gmtime() shares one static buffer; both calls were aliased.
    // Use gmtime_r() with separate stack structs.
    struct tm startTm, endTm;
    gmtime_r(&startTime, &startTm);
    gmtime_r(&endTime,   &endTm);
    char timeStr[12];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d-%02d:%02d",
             startTm.tm_hour, startTm.tm_min,
             endTm.tm_hour,   endTm.tm_min);
    return String(timeStr);
}

// Helper: Truncate event title to fit display
String CalendarManager::truncateEventTitle(const String& title, int maxLength) const {
    if (title.length() <= maxLength) {
        return title;
    }
    return title.substring(0, maxLength - 3) + "...";
}

// Check if calendar should be updated today
bool CalendarManager::shouldUpdateToday() const {
    if (lastUpdateDay == -1) {
        return true;
    }

    time_t now = timeClient.getEpochTime();
    if (now < 946684800) {
        LOG_DEBUG_F("Time not synchronized, will update calendar when time is available");
        return true;
    }

    struct tm tmValue;
    if (gmtime_r(&now, &tmValue) == nullptr) {
        LOG_WARNING_F("Failed to get time info, will retry calendar update");
        return true;
    }

    return (lastUpdateDay != tmValue.tm_mday);
}

// Mark calendar as updated for today
void CalendarManager::markUpdated() {
    time_t now = timeClient.getEpochTime();
    if (now < 946684800) {
        LOG_WARNING_F("Time not synchronized, cannot mark calendar as updated");
        return;
    }

    struct tm tmValue;
    if (gmtime_r(&now, &tmValue) != nullptr) {
        lastUpdateDay = tmValue.tm_mday;
        LOG_INFO_FMT("Calendar marked as updated for day %d", lastUpdateDay);
    } else {
        LOG_WARNING_F("Failed to get time info, cannot mark calendar as updated");
    }
}
