#include "frame.h"

#include <stdio.h>

#include "state.h"
#include "serial_port.h"
#include "special_bytes.h"
#include "link_layer.h"

int readKnownSOrUFrame(uint8_t addressField, uint8_t controlField) {
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

            state = nextKnownSOrUFrameState(state, byte, addressField, controlField);
        }
    }

    printf(": %d bytes\n", totalBytes);
    return 1;
}

int readKnownSOrUFrameTimeout(uint8_t addressField, uint8_t controlField) {
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

            state = nextKnownSOrUFrameState(state, byte, addressField, controlField);
        }
    }

    printf(": %d bytes\n", totalBytes);
    return alarmStatus.enabled ? 1 : 0;
}

int readSOrUFrameTimeout(uint8_t addressField, uint8_t *controlField) {
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

            state = nextSOrUFrameState(state, byte, addressField, controlField);
        }
    }

    printf(": %d bytes\n", totalBytes);
    return alarmStatus.enabled ? 1 : 0;
}

int readIFrame(uint8_t addressField, uint8_t *frameNumber, uint8_t* data) {
    State state = STATE_START;
    uint8_t xor = 0;
    int dataIndex = -1;
    int totalBytes = 0;

    printf("Read  <- ");

    uint8_t byte;
    uint8_t prevByte;
    while (state != STATE_STOP) {
        int r = readByteSerialPort(&byte);
        if (r < 0)
            return -1;

        if (r == 1) {
            if (state == STATE_DATA) {
                if (dataIndex > 0)
                    data[dataIndex] = prevByte;

                dataIndex++;
                prevByte = byte;

            } else {
                dataIndex = -1;
            }

            totalBytes++;
            printf(":%02x", byte);
            fflush(stdout);

            state = nextIFrameState(state, byte, addressField, frameNumber, &xor);

            if (state == STATE_BCC2_BAD)
                return -1;
        }
    }

    printf(": %d bytes\n", totalBytes);
    return alarmStatus.enabled ? dataIndex : 0;
}


int writeFrame(const uint8_t *frame, int frameSize) {
    printf("Write -> ");

    int bytes;
    for (int i = 0; i < frameSize; i += bytes) {
        bytes = writeBytesSerialPort(frame + i, frameSize);
        if (bytes < 0)
            return -1;

        for (int j = 0; j < bytes; j++)
            printf(":%02x", frame[i + j]);
    }

    printf(": %d bytes\n", frameSize);
    return bytes;
}

int writeSOrUFrame(uint8_t addressField, uint8_t controlField) {
    uint8_t frame[SU_FRAME_SIZE] = {FLAG, addressField, controlField, addressField ^ controlField, FLAG};

    return writeFrame(frame, SU_FRAME_SIZE);
}

int writeIFrame(uint8_t addressField, uint8_t frameNumber, const uint8_t *data, int dataSize) {
    uint8_t frame[I_FRAME_BASE_SIZE + MAX_PAYLOAD_SIZE];

    frame[0] = FLAG;
    frame[1] = addressField;
    frame[2] = C(frameNumber);
    frame[3] = addressField ^ C(frameNumber);
    
    uint8_t bcc2 = 0;
    for (int i = 0; i < dataSize && i < MAX_PAYLOAD_SIZE; i++) {
        frame[4 + i] = data[i];
        bcc2 ^= data[i];
    }
    
    frame[4 + dataSize] = bcc2;
    frame[4 + dataSize + 1] = FLAG;

    if (writeFrame(frame, I_FRAME_BASE_SIZE + dataSize) < 0)
        return -1;
    return dataSize;
}
