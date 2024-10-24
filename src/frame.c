#include "frame.h"

#include <stdio.h>

#include "state.h"
#include "serial_port.h"
#include "special_bytes.h"
#include "link_layer.h"

int readSOrUFrame(uint8_t addressField, uint8_t *controlField, int timeout) {
    State state = STATE_START;
    int totalBytes = 0;

    printf("Read  <- ");
    while (state != STATE_STOP && (!timeout || alarmStatus.enabled)) {
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
    return !timeout || alarmStatus.enabled ? 1 : 0;
}

uint8_t decodeByte(uint8_t byte) {
    return byte == ESC2_FLAG ? FLAG : ESC;
}

int readIFrame(uint8_t addressField, uint8_t *frameNumber, uint8_t *data) {
    State state = STATE_START;
    uint8_t xor = 0;
    int dataIndex = -1;
    int totalBytes = 0;

    printf("Read  <- ");

    uint8_t byte;
    uint8_t prevByte;
    while (state != STATE_STOP && state != STATE_BCC2_BAD && state != STATE_STUFF_BAD && state != STATE_STOP_SET && state != STATE_STOP_DISC) {
        int r = readByteSerialPort(&byte);
        if (r < 0)
            return -1;

        if (r == 1) {
            totalBytes++;
            printf(":%02x", byte);
            fflush(stdout);

            state = nextIFrameState(state, byte, addressField, frameNumber, xor);

            if (isDataState(state)) {
                if (dataIndex >= 0) {
                    if (state == STATE_DATA_WRT_STUFF || state == STATE_DATA_ESC_WRT_STUFF) {
                        if (dataIndex >= MAX_PAYLOAD_SIZE)
                            return -2;
                        data[dataIndex] = decodeByte(prevByte);
                    } else if (state != STATE_DATA_STUFF) {
                        if (dataIndex >= MAX_PAYLOAD_SIZE)
                            return -2;
                        data[dataIndex] = prevByte;
                    }
                }

                if (state == STATE_DATA_STUFF) {
                    xor ^= decodeByte(byte);
                } else if (state == STATE_DATA || state == STATE_DATA_WRT_STUFF) {
                    xor ^= byte;
                    dataIndex++;
                } else {  // STATE_DATA_ESC and STATE_DATA_ESC_WRT_STUFF
                    dataIndex++;
                }

                prevByte = byte;

            } else if (state != STATE_STOP) {
                dataIndex = -1;
            }
        }
    }

    printf(": %d bytes\n", totalBytes);

    if (state == STATE_BCC2_BAD || state == STATE_STUFF_BAD)
        return -2;  // Data error
    if (state == STATE_STOP_SET)
        return -3;
    if (state == STATE_STOP_DISC)
        return -4;

    return dataIndex;
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

int putByte(uint8_t byte, int *index, uint8_t *frame) {
    if (byte == FLAG) {
        if (*index + 1 >= 2 * MAX_PAYLOAD_SIZE)
            return -1;

        frame[4 + *index] = ESC;
        frame[4 + *index + 1] = ESC2_FLAG;
        *index += 2;

        return 1;
    }
    
    if (byte == ESC) {
        if (*index + 1 >= 2 * MAX_PAYLOAD_SIZE)
            return -1;

        frame[4 + *index] = ESC;
        frame[4 + *index + 1] = ESC2_ESC;
        *index += 2;

        return 1;
    }
    
    frame[4 + *index] = byte;
    (*index)++;

    return 1;
}

int writeIFrame(uint8_t addressField, uint8_t frameNumber, const uint8_t *data, int dataSize) {
    uint8_t frame[I_FRAME_BASE_SIZE + 2 * (MAX_PAYLOAD_SIZE + 1)];  // Payload and BCC2 can be byte stuffed

    frame[0] = FLAG;
    frame[1] = addressField;
    frame[2] = C(frameNumber);
    frame[3] = addressField ^ C(frameNumber);
    
    uint8_t bcc2 = 0;
    int frameIndex = 0;
    for (int dataIndex = 0; dataIndex < dataSize && frameIndex < 2 * MAX_PAYLOAD_SIZE; dataIndex++) {
        bcc2 ^= data[dataIndex];
        putByte(data[dataIndex], &frameIndex, frame);
    }

    int bccSize = 0;
    putByte(bcc2, &bccSize, frame + frameIndex);
    frame[4 + frameIndex + bccSize] = FLAG;

    if (writeFrame(frame, I_FRAME_BASE_SIZE + frameIndex + bccSize) < 0)
        return -1;
    return frameIndex;
}
