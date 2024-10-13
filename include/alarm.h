#ifndef _ALARM_H_
#define _ALARM_H_

typedef struct {
    int enabled;
    int count;
} AlarmStatus;

extern AlarmStatus alarmStatus;

void configAlarm();
void resetAlarm(unsigned int timeout);
void stopAlarm();

#endif  // _ALARM_H_
