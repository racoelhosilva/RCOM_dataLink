#include "alarm.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

AlarmStatus alarmStatus;

// Alarm function handler
void alarmHandler(int signal) {
    alarmStatus.enabled = 0;
    alarmStatus.count++;
}

void configAlarm() {
    alarmStatus.enabled = 0;
    alarmStatus.count = 0;

    (void)signal(SIGALRM, alarmHandler);
}

void resetAlarm(unsigned int timeout) {
    alarm(timeout);
    alarmStatus.enabled = 1;
}

void stopAlarm() {
    alarm(0);
    alarmStatus.enabled = 0;
}
