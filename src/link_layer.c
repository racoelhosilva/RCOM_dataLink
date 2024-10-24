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

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

LinkLayer conParams;
uint8_t frameNumber;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    conParams = connectionParameters;

    const char* serialPort = conParams.serialPort;
    int baudRate = conParams.baudRate;
    if (openSerialPort(serialPort, baudRate) < 0)
        return -1;

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

            // Wait until all bytes have been written to the serial port
            // sleep(1);  // TODO: Is this to be removed?

            // TODO: Respond to DISC
            uint8_t controlField;
            int r;
            while (TRUE) {
                r = readSOrUFrame(A1, &controlField, TRUE);
                if (r < 0 || (r > 0 && controlField == UA)) {
                    stopAlarm();
                    return r;
                }
            }

            printf("Try #%d\n", alarmStatus.count);
        }

        stopAlarm();
        perror("llopen");
        return -1;

    } else {
        uint8_t controlField;
        while (TRUE) {
            if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                return -1;

            if (controlField == SET) {
                return writeSOrUFrame(A1, UA);
            }
            if (controlField == DISC) {
                if (writeSOrUFrame(A1, DISC) < 0)
                    return -1;

                do {
                    if (readSOrUFrame(A1, &controlField, FALSE) < 0)
                        return -1;
                } while (controlField != UA);

                return -1;
            }
        }
    }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
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
            continue;
        }

        printf("Try #%d\n", alarmStatus.count);
    }

    perror("llwrite");
    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
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
            if (writeSOrUFrame(A1, REJ(frameNumber)) < 0)
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

            perror("llread");
            return -1;
        }
    }

    perror("llread");
    return -1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
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
                printf("Try #%d\n", alarmStatus.count);
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
    }

    int clstat = closeSerialPort();
    return clstat;
}
