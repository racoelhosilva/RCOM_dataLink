#ifndef _ALARM_H_
#define _ALARM_H_

typedef struct {
    int enabled;
    int count;
} AlarmStatus;

extern AlarmStatus alarmStatus;

// Configures the alarm and initiates its values.
void configAlarm();

// Restarts the alarm to activate after the specified seconds.
void resetAlarm(unsigned int timeout);

// Stops the alarm.
void stopAlarm();

#endif  // _ALARM_H_
