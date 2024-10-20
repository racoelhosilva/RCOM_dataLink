#include "packet.h"

#include <string.h>
#include <stdio.h>

#include "link_layer.h"

int readControlPacket(uint8_t* controlField, uint32_t *filesize, char *filename) {
    uint8_t packet[BASE_CONTROL_PACKET_SIZE + MAX_FILENAME_SIZE];
    int T1, L1, T2, L2;

    int bytes = llread(packet);
    if (bytes < 0)
        return -1;
    if (bytes < BASE_CONTROL_PACKET_SIZE) {
        perror("readControlPacket");
        return -1;
    }
        
    *controlField = packet[0];
    if (*controlField != 1 && *controlField != 3) {
        perror("readControlPacket");
        return -1;
    }

    // TLV for File Size
    T1 = packet[1];
    if (T1 != 0) {
        perror("readControlPacket");
        return -1;
    }

    L1 = packet[2];
    if (L1 != 4) {
        perror("readControlPacket");
        return -1;
    }

    memcpy(filesize, packet + 3, 4);

    // TLV for File Name
    T2 = packet[7];
    if (T2 != 1) {
        perror("readControlPacket");
        return -1;
    }
    
    L2 = packet[8];
    memcpy(filename, packet + 9, L2);
    filename[L2] = '\0';

    return 1;
}

uint8_t fromBcd(uint8_t num) {
    return ((num >> 4) * 10) + (num & 0x0F);
}

int readDataPacket(uint8_t *controlField, uint8_t *sequenceNumber, uint8_t *packetData) {
    uint8_t packet[BASE_DATA_PACKET_SIZE + MAX_DATA_PACKET_PAYLOAD_SIZE];

    int bytes = llread(packet);
    if (bytes < 0)
        return -1;

    *controlField = packet[0];
    if (*controlField != 2) {
        perror("readDataPacket");
        return -1;
    }
    
    *sequenceNumber = fromBcd(packet[1]);
    
    uint16_t packetDataSize = (packet[2] << 8) + packet[3];
    memcpy(packetData, packet + 4, packetDataSize);
    
    return packetDataSize;
}

int writeControlPacket(uint8_t controlField, uint32_t filesize, char *filename) {
    int filenameSize = strlen(filename);
    if (filenameSize > MAX_FILENAME_SIZE) {
        return -1;
    }

    uint32_t bufSize = 1 + 1 + 1 + 4 + 1 + 1 + filenameSize;
    uint8_t buf[bufSize];

    buf[0] = controlField;

    // TLV for file size
    buf[1] = 0;
    buf[2] = 4;
    memcpy(buf + 3, &filesize, 4);

    // TLV for file name
    buf[7] = 1;
    buf[8] = filenameSize;
    memcpy(buf + 9, filename, filenameSize);

    return llwrite(buf, bufSize);
}

uint8_t toBcd(uint8_t num) {
    return ((num / 10) << 4) + (num % 10);
}

int writeDataPacket(uint8_t sequenceNumber, uint16_t packetDataSize, uint8_t *packetData) {
    if (packetDataSize > MAX_DATA_PACKET_PAYLOAD_SIZE) {
        return -1;
    }

    uint32_t bufSize = 1 + 1 + 2 + packetDataSize;
    uint8_t buf[bufSize];
    uint32_t index = 0;

    buf[index++] = 2;
    buf[index++] = toBcd(sequenceNumber);
    buf[index++] = (packetDataSize >> 8) & 0xFF;
    buf[index++] = (packetDataSize) & 0xFF;
    
    memcpy(buf + index, packetData, packetDataSize);

    return llwrite(buf, bufSize);
}
