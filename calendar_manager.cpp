#include "calendar_manager.h"
#include "logger.h"
#include "TimeManager.h"
#include "global_config.h"
#include "led_display.h"
#include "device_state.h"
#include <TimeLib.h>

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
    // Empty string means event applies to all boards
    if (eventHostname == nullptr || strlen(eventHostname) == 0) {
        return true;
    }
    
    // Get current board hostname
    String currentHostname = DeviceState::getInstance().getHostname();
    
    // Compare hostnames
    return (currentHostname == String(eventHostname));
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
    
    struct tm* timeinfo = gmtime(&now);
    if (timeinfo == nullptr) {
        return;
    }
    
    int currentMonth = timeinfo->tm_mon + 1; // tm_mon is 0-11
    int currentDay = timeinfo->tm_mday;
    int currentHour = timeinfo->tm_hour;
    int currentYear = timeinfo->tm_year + 1900;
    
    const BirthdayEvent* nextEvent = nullptr;
    time_t nextEventStartTimeValue = 0;
    time_t nextEventEndTimeValue = 0;
    int minDaysAhead = 366; // More than a year
    
    // Get current board hostname for priority checking
    String currentHostname = DeviceState::getInstance().getHostname();
    
    // Find the next event within next 365 days
    for (int i = 0; i < calendarEventsCount; i++) {
        const BirthdayEvent* event = &calendarEvents[i];
        
        // Skip events that don't match this board's hostname
        if (!matchesHostname(event->hostname)) {
            continue;
        }
        
        // If this is a generic event (empty hostname), check if there's a specific event for this date
        // If specific event exists, skip this generic event (specific has priority)
        // This means: "for all boards" = "for all boards EXCEPT those with specific event on this date"
        if (event->hostname == nullptr || strlen(event->hostname) == 0) {
            // Check if there's a specific event for this date for current board
            bool hasSpecificEventForThisDate = false;
            for (int j = 0; j < calendarEventsCount; j++) {
                const BirthdayEvent* checkEvent = &calendarEvents[j];
                if (checkEvent->hostname != nullptr && 
                    strlen(checkEvent->hostname) > 0 &&
                    currentHostname == String(checkEvent->hostname) &&
                    checkEvent->month == event->month &&
                    checkEvent->day == event->day) {
                    hasSpecificEventForThisDate = true;
                    break;
                }
            }
            if (hasSpecificEventForThisDate) {
                continue; // Skip generic event - specific event exists for this date
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
        
        // Use board-specific title if available, otherwise use general title
        if (nextEvent->boardTitle != nullptr && strlen(nextEvent->boardTitle) > 0) {
            nextEventTitle = String(nextEvent->boardTitle);
        } else {
            nextEventTitle = String(nextEvent->title);
        }
        
        nextEventStartTime = nextEventStartTimeValue;
        nextEventEndTime = nextEventEndTimeValue;
        
        // Format time display
        if (nextEvent->fromHour >= 0) {
            if (nextEvent->toHour >= 0 && nextEvent->toHour != nextEvent->fromHour) {
                // Time range
                nextEventTime = formatEventTimeRange(nextEventStartTime, nextEventEndTime);
            } else {
                // Single time
                nextEventTime = formatEventTime(nextEventStartTime);
            }
        } else {
            nextEventTime = String("All day");
        }
        
        LOG_INFO("Next event found: " + nextEventTitle + " on " + 
                 String(nextEvent->month) + "-" + String(nextEvent->day) + 
                 (nextEvent->fromHour >= 0 ? (" at " + nextEventTime) : " (all day)") +
                 " (in " + String(minDaysAhead) + " days)");
    }
}

// Function to read calendar events from birthdays.h
void CalendarManager::readCalendarEvents() {
    LOG_INFO_F("Reading calendar events from birthdays.h...");
    
    // Find next upcoming event
    findNextEvent();
    
    // Mark as updated
    markUpdated();
    
    if (hasEvent) {
        LOG_INFO("Calendar event loaded: " + nextEventTitle);
    } else {
        LOG_INFO_F("No upcoming events found");
    }
}

// Check if event should be displayed now (once per 15 minutes)
bool CalendarManager::shouldDisplayNow() const {
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
void CalendarManager::printNextEventToScreen() const {
    if (!hasEvent || nextEventTitle.length() == 0) {
        String tape = "No events";
        LOG_INFO(">> Display: Calendar = " + tape);
        drawString(tape);
        // Update last display time even for "No events"
        const_cast<CalendarManager*>(this)->lastDisplayTime = millis();
        return;
    }
    
    // Check if event is today - only show events on the day of the event
    time_t now = timeClient.getEpochTime();
    struct tm* timeinfo = gmtime(&now);
    if (timeinfo == nullptr) {
        printTimeToScreen();
        return;
    }
    
    int currentMonth = timeinfo->tm_mon + 1;
    int currentDay = timeinfo->tm_mday;
    
    // Get event date from stored start time
    struct tm* eventTimeinfo = gmtime(&nextEventStartTime);
    if (eventTimeinfo == nullptr) {
        printTimeToScreen();
        return;
    }
    
    int eventMonth = eventTimeinfo->tm_mon + 1;
    int eventDay = eventTimeinfo->tm_mday;
    
    bool isEventToday = (eventMonth == currentMonth && eventDay == currentDay);
    
    // Only show events that are today
    if (!isEventToday) {
        LOG_DEBUG("Event is not today (current: " + String(currentMonth) + "/" + String(currentDay) + 
                  ", event: " + String(eventMonth) + "/" + String(eventDay) + "), showing time instead");
        printTimeToScreen();
        return;
    }
    
    // Event is today - check if it's currently active (for timed events)
    bool isActive = isEventActiveNow();
    
    LOG_DEBUG("Event is today, isActive=" + String(isActive ? "true" : "false"));
    
    // For timed events, only show when active. For all-day events, show all day
    // Check if we should display now (once per 15 minutes)
    // But always display if event is currently active (or if it's an all-day event)
    if (!shouldDisplayNow() && !isActive) {
        // Skip display, show time instead
        printTimeToScreen();
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
    
    LOG_INFO(">> Display: Calendar Event = " + tape + (isActive ? " (ACTIVE)" : ""));
    drawString(tape);
    
    // Update last display time
    const_cast<CalendarManager*>(this)->lastDisplayTime = millis();
}

// Helper: Format event time as HH:MM
String CalendarManager::formatEventTime(time_t eventTime) const {
    struct tm* timeinfo = gmtime(&eventTime);
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    return String(timeStr);
}

// Helper: Format event time range as "HH:MM-HH:MM"
String CalendarManager::formatEventTimeRange(time_t startTime, time_t endTime) const {
    struct tm* startInfo = gmtime(&startTime);
    struct tm* endInfo = gmtime(&endTime);
    char timeStr[12];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d-%02d:%02d", 
             startInfo->tm_hour, startInfo->tm_min,
             endInfo->tm_hour, endInfo->tm_min);
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
    // If never updated, always return true
    if (lastUpdateDay == -1) {
        return true;
    }
    
    // Check if time is valid before using it
    time_t now = timeClient.getEpochTime();
    if (now < 946684800) { // Check if time is valid (after 2000-01-01)
        LOG_DEBUG_F("Time not synchronized, will update calendar when time is available");
        return true; // Update when time becomes available
    }
    
    struct tm* timeinfo = gmtime(&now);
    if (timeinfo == nullptr) {
        LOG_WARNING_F("Failed to get time info, will retry calendar update");
        return true;
    }
    
    int currentDay = timeinfo->tm_mday;
    
    // Update if day changed
    return (lastUpdateDay != currentDay);
}

// Mark calendar as updated for today
void CalendarManager::markUpdated() {
    time_t now = timeClient.getEpochTime();
    if (now < 946684800) { // Check if time is valid (after 2000-01-01)
        LOG_WARNING_F("Time not synchronized, cannot mark calendar as updated");
        return;
    }
    
    struct tm* timeinfo = gmtime(&now);
    if (timeinfo != nullptr) {
        lastUpdateDay = timeinfo->tm_mday;
        LOG_INFO("Calendar marked as updated for day " + String(lastUpdateDay));
    } else {
        LOG_WARNING_F("Failed to get time info, cannot mark calendar as updated");
    }
}
