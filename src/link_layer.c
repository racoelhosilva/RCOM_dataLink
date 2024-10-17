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

int maxTries;
int timeout;
uint8_t frameNumber;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    const char* serialPort = connectionParameters.serialPort;
    int baudRate = connectionParameters.baudRate;
    if (openSerialPort(serialPort, baudRate) < 0)
        return -1;

    LinkLayerRole role = connectionParameters.role;
    maxTries = connectionParameters.nRetransmissions;
    timeout = connectionParameters.timeout;
    frameNumber = 0;

    if (role == LlTx) {
        configAlarm();

        for (int nTries = 0; nTries < maxTries; nTries++) {
            stopAlarm();
            printf("Try #%d\n", nTries);

            if (writeSOrUFrame(A1, SET) < 0)
                return -1;

            resetAlarm(timeout);

            // Wait until all bytes have been written to the serial port
            // sleep(1);  // TODO: Is this to be removed?

            int r = readKnownSOrUFrameTimeout(A1, UA);
            if (r != 0) {
                stopAlarm();
                return r;
            }
        }

        stopAlarm();
        perror("llopen");
        return -1;

    } else {
        if (readKnownSOrUFrame(A1, SET) < 0)
            return -1;
        if (writeSOrUFrame(A1, UA) < 0)
            return -1;

        // sleep(1);  // TODO: Remove this?
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    configAlarm();

    for (int nTries = 0; nTries < maxTries; nTries++) {
        stopAlarm();
        printf("Try #%d\n", nTries);

        int bytes = writeIFrame(A1, frameNumber, buf, bufSize);
        if (bytes < 0)
            return -1;

        resetAlarm(timeout);

        // Wait until all bytes have been written to the serial port
        // sleep(1);  // TODO: Is this to be removed?

        uint8_t controlField = 0;

        int r;
        if ((r = readSOrUFrameTimeout(A1, &controlField)) < 0)
            return -1;

        int goodControlField = controlField != RR(0) && controlField != RR(1) && controlField != REJ(frameNumber);
        if (r == 0 || goodControlField)
            continue;

        if (controlField == RR(!frameNumber)) {
            frameNumber = !frameNumber;
            return bytes;
        }
    }

    stopAlarm();
    perror("llwrite");
    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    int bytes;
    uint8_t readFrameNumber;

    for (int nTries = 0; nTries < maxTries; nTries++) {
        bytes = readIFrame(A1, &readFrameNumber, packet);
        if (bytes > 0 && readFrameNumber == frameNumber) {
            int r = writeSOrUFrame(A1, RR(!frameNumber));
            if (r < 0)
                return -1;

            frameNumber = !frameNumber;
            return bytes;

        } else if (bytes == 0) {
            if (writeSOrUFrame(A1, REJ(frameNumber)) < 0)
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
    // TODO

    int clstat = closeSerialPort();
    return clstat;
}
