#include "frame.h"

#include <stdio.h>

#include "state.h"
#include "serial_port.h"
#include "special_bytes.h"

int readSOrUFrame(uint8_t addressField, uint8_t controlField) {
    State state = STATE_START;
    int totalBytes = 0;

    printf("Read  <- ");
    while (state != STATE_STOP) {
        uint8_t byte;
        int r = readByteSerialPort(&byte);
        if (r < 0)
            return -1;

        if (r == 1) {
            totalBytes++;
            printf(":%02x", byte);

            state = nextSOrUFrameState(state, byte, addressField, controlField);
        }
    }

    printf(": %d bytes\n", totalBytes);
    return 1;
}


int readSOrUFrameTimeout(uint8_t addressField, uint8_t controlField) {
    State state = STATE_START;
    int totalBytes = 0;

    printf("Read  <- ");
    while (state != STATE_STOP && alarmStatus.enabled) {
        uint8_t byte;
        int r = readByteSerialPort(&byte);
        if (r < 0)
            return -1;

        if (r == 1) {
            totalBytes++;
            printf(":%02x", byte);
            fflush(stdout);

            state = nextSOrUFrameState(state, byte, addressField, controlField);
        }
    }

    printf(": %d bytes\n", totalBytes);
    return alarmStatus.enabled ? 1 : 0;
}


int writeFrame(uint8_t *frame, int frameSize) {
    printf("Write -> ");

    int bytes = 0;
    for (int i = 0; i < frameSize; i += bytes) {
        bytes = writeBytesSerialPort(frame + i, frameSize);
        if (bytes < 0)
            return -1;

        for (int j = 0; j < bytes; j++)
            printf(":%02x", frame[i + j]);
    }

    printf(": %d bytes\n", frameSize);
    return 1;
}

int writeSOrUFrame(uint8_t addressField, uint8_t controlField) {
    uint8_t buf[FRAME_BASE_SIZE] = {FLAG, addressField, controlField, addressField ^ controlField, FLAG};

    return writeFrame(buf, FRAME_BASE_SIZE);
}