#include <string.h>
#include "packet.h"

int readControlPacket() {
    
    return 1;
}

int writeControlPacket(uint8_t controlField, uint32_t filesize, uint8_t filenameSize, char *filename) {
    if (filenameSize > MAX_FILENAME_SIZE) {
        return -1;
    }

    uint32_t bufSize = 1 + 1 + 1 + 4 + 1 + 1 + filenameSize;
    uint8_t buf[bufSize];
    uint32_t index = 0;

    buf[index++] = controlField;

    // TLV for File Size
    buf[index++] = 0;
    buf[index++] = 4;
    buf[index++] = (filesize >> 24) & 0xFF;
    buf[index++] = (filesize >> 16) & 0xFF;
    buf[index++] = (filesize >> 8) & 0xFF;
    buf[index++] = (filesize) & 0xFF;

    // TLV for File Name
    buf[index++] = 1;
    buf[index++] = filenameSize;
    memcpy(&buf[index], filename, filenameSize);

    return 1;
}

int writeDataPacket(uint8_t sequenceNumber, uint16_t packetDataSize, uint8_t *packetData) {
    if (packetDataSize > MAX_DATA_PACKET_SIZE) {
        return -1;
    }

    uint32_t bufSize = 1 + 1 + 2 + packetDataSize;
    uint8_t buf[bufSize];
    uint32_t index = 0;

    buf[index++] = 2;
    buf[index++] = sequenceNumber;
    buf[index++] = (packetDataSize >> 8) & 0xFF;
    buf[index++] = (packetDataSize) & 0xFF;
    
    memcpy(&buf[index], packetData, packetDataSize);

    return 1;
}
