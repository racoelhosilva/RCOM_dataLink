// Link layer protocol implementation

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "link_layer.h"
#include "serial_port.h"
#include "special_bytes.h"
#include "state.h"
#include "frame.h"
#include "alarm.h"
#include "debug.h"
#include "statistics.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int connectionOpen;
LinkLayer conParams;
uint8_t frameNumber;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    if (connectionOpen)
        return -1;
    conParams = connectionParameters;

    const char* serialPort = conParams.serialPort;
    int baudRate = conParams.baudRate;
    if (openSerialPort(serialPort, baudRate) < 0)
        return -1;
    
    initStatistics(&conParams);
    LinkLayerRole role = conParams.role;
    int maxTries = conParams.nRetransmissions;
    int timeout = conParams.timeout;
    frameNumber = 0;

    if (role == LlTx) {
        configAlarm();

        while (alarmStatus.count < maxTries) {
            stopAlarm();

            if (writeSOrUFrame(A1, SET) < 0)
                return -1;

            resetAlarm(timeout);
            
            uint8_t controlField;
            int r;
            do {
                r = readSOrUFrame(A1, &controlField, TRUE);
                if (r < 0) {
                    stopAlarm();
                    return -1;
                }
            } while (r > 0 && controlField != UA);

            if (r > 0) {
                stopAlarm();
                connectionOpen = TRUE;
                clock_gettime(CLOCK_MONOTONIC, &statistics.start);
                return 1;
            }

            statistics.totalTimeouts++;
            debugLog("Try #%d\n", alarmStatus.count);
        }

        stopAlarm();
        errorLog(__func__, "Max connection tries exceeded");
        return -1;

    } else {
        uint8_t controlField;
        while (TRUE) {
            if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                return -1;

            if (controlField == SET) {
                if (writeSOrUFrame(A1, UA) < 0)
                    return -1;
                
                connectionOpen = TRUE;
                clock_gettime(CLOCK_MONOTONIC, &statistics.start);
                return 1;
            }
            if (controlField == DISC) {
                if (writeSOrUFrame(A1, DISC) < 0)
                    return -1;

                do {
                    if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                        return -1;
                } while (controlField != UA);

                connectionOpen = FALSE;
                errorLog(__func__, "Connection forcefully closed");
                return -1;
            }
        }
    }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    if (!connectionOpen || conParams.role != LlTx) {
        errorLog(__func__, "No connection open or called as receiver");
        return -1;
    }
    if (buf == NULL) {
        errorLog(__func__, "Null pointer passed as parameter");
        return -1;
    }

    int maxTries = conParams.nRetransmissions;
    int timeout = conParams.timeout;

    configAlarm();

    while (alarmStatus.count < maxTries) {
        int bytes = writeIFrame(A1, frameNumber, buf, bufSize);
        if (bytes < 0)
            return -1;

        resetAlarm(timeout);

        uint8_t controlField;
        int r, badControlField;
        do {
            r = readSOrUFrame(A1, &controlField, TRUE);
            if (r < 0) {
                stopAlarm();
                return -1;
            }

            badControlField = controlField != RR(0) && controlField != RR(1) && controlField != REJ(frameNumber);
        } while (r > 0 && badControlField);

        if (r > 0) {
            if (controlField == RR(!frameNumber)) {
                frameNumber = !frameNumber;
                stopAlarm();
                return bytes;
            }

            alarmStatus.count = 0;
            stopAlarm();
            statistics.totalRej++;
            continue;
        }

        statistics.totalTimeouts++;
        debugLog("Try #%d\n", alarmStatus.count);
    }

    errorLog(__func__, "Max send tries exceeded");
    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    if (!connectionOpen || conParams.role != LlRx) {
        errorLog(__func__, "No connection open or called as transmitter");
        return -1;
    }
    if (packet == NULL) {
        errorLog(__func__, "Null pointer passed as parameter");
        return -1;
    }

    int bytes;
    uint8_t readFrameNumber;

    while (TRUE) {
        bytes = readIFrame(A1, &readFrameNumber, packet);

        if (bytes > 0) {
            if (readFrameNumber == frameNumber) {
                if (writeSOrUFrame(A1, RR(!frameNumber)) < 0)
                    return -1;

                frameNumber = !frameNumber;
                return bytes;

            } else if (writeSOrUFrame(A1, RR(frameNumber)) < 0) {
                return -1;
            }

        } else if (bytes == -2) {  // Data error
            if (writeSOrUFrame(A1, readFrameNumber == frameNumber ? REJ(frameNumber) : RR(frameNumber)) < 0)
                return -1;

        } else if (bytes == -3) {  // SET received
            if (writeSOrUFrame(A1, UA) < 0)
                return -1;

        } else if (bytes == -4) {  // DISC received
            uint8_t controlField;
            if (writeSOrUFrame(A1, DISC) < 0)
                return -1;

            do {
                if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                    return -1;
            } while (controlField != UA);

            errorLog(__func__, "Connection forcefully closed");
            connectionOpen = FALSE;
            return -1;
        }
    }

    return -1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    if (!connectionOpen) {
        errorLog(__func__, "No connection open");
        return -1;
    }

    LinkLayerRole role = conParams.role;
    int maxTries = conParams.nRetransmissions;
    int timeout = conParams.timeout;

    if (role == LlTx) {
        configAlarm();

        while (alarmStatus.count < maxTries) {
            stopAlarm();

            int bytes = writeSOrUFrame(A1, DISC);
            if (bytes < 0)
                return -1;

            resetAlarm(timeout);

            uint8_t controlField;
            int r;
            do {
                if ((r = readSOrUFrame(A1, &controlField, TRUE)) < 0) {
                    stopAlarm();
                    return -1;
                }
            } while (r > 0 && controlField != DISC);
            
            if (r == 0) {
                statistics.totalTimeouts++;
                debugLog("Try #%d\n", alarmStatus.count);
                continue;
            }

            bytes = writeSOrUFrame(A1, UA);
            if (bytes < 0) {
                stopAlarm();
                return -1;
            }
            break;
        }

        stopAlarm();
        clock_gettime(CLOCK_MONOTONIC, &statistics.end);
        
    } else {
        uint8_t controlField;
        do {
            if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                return -1;
        } while (controlField != DISC);

        if (writeSOrUFrame(A1, DISC) < 0)
            return 1;

        do {
            if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                return -1;
        } while (controlField != UA);
        
        clock_gettime(CLOCK_MONOTONIC, &statistics.end);
    }

    connectionOpen = FALSE;
    
    if (showStatistics){
        printStatistics();
        storeStatistics();
    }

    int clstat = closeSerialPort();
    return clstat;
}
