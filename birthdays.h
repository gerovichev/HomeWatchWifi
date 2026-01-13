#ifndef BIRTHDAYS_H
#define BIRTHDAYS_H

// Calendar events and birthdays
// Format: {month, day, from hour, to hour, "Title", "Title of specific board", "Host name of the board"}
// from hour = -1 for all-day events
// from hour = 0-23 for timed events
// to hour = -1 means event ends at 23:59
// to hour = 0-23 for events with end time
// If hostname is empty string "", event applies to all boards EXCEPT boards with specific event on this date
// Priority: specific events (with hostname) have priority over generic events (empty hostname) on the same date

struct BirthdayEvent {
    int month;              // 1-12
    int day;                // 1-31
    int fromHour;           // -1 for all-day, 0-23 for timed events (start hour)
    int toHour;             // -1 for all-day (ends at 23:59), 0-23 for timed events (end hour)
    const char* title;       // General title for all boards
    const char* boardTitle; // Specific title for this board (can be empty)
    const char* hostname;   // Host name of the board (empty "" for all boards)
};

// Declare events array (defined in BIRTHDAYS.cpp)
extern const BirthdayEvent calendarEvents[];
extern const int calendarEventsCount;

#endif // BIRTHDAYS_H
