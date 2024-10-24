#include "packet.h"

#include <string.h>
#include <stdio.h>

#include "link_layer.h"
#include "special_bytes.h"
#include "debug.h"

int readControlPacket(uint8_t* controlField, uint32_t *filesize, char *filename) {
    uint8_t packet[MAX_PAYLOAD_SIZE];
    int T1, L1, T2, L2;

    int bytes = llread(packet);
    if (bytes < 0)
        return -1;
    if (bytes < BASE_CONTROL_PACKET_SIZE) {
        errorLog(__func__, "Packet too small to be control packet");
        return -1;
    }
        
    *controlField = packet[0];
    if (*controlField != CF_START && *controlField != CF_END) {
        errorLog(__func__, "Control field mismatch (received %d, expected %d or %d)", controlField, CF_START, CF_END);
        return -1;
    }

    // TLV for File Size
    T1 = packet[1];
    if (T1 != TLV_FILESIZE) {
        errorLog(__func__, "Unexpected parameter type (received %d, expected %d)", T1, TLV_FILESIZE);
        return -1;
    }

    L1 = packet[2];
    if (L1 != 4) {
        errorLog(__func__, "Unexpected parameter length (received %d, expected %d)", L1, 4);
        return -1;
    }

    *filesize = (packet[3] << 24) + (packet[4] << 16) + (packet[5] << 8) + packet[6];

    // TLV for File Name
    T2 = packet[7];
    if (T2 != TLV_FILENAME) {
        errorLog(__func__, "Unexpected parameter type (received %d, expected %d)", T2, TLV_FILENAME);
        return -1;
    }
    
    L2 = packet[8];
    if (L2 > MAX_FILENAME_SIZE) {
        errorLog(__func__, "Max filename size exceeded");
        return -1;
    }
    memcpy(filename, packet + 9, L2);
    filename[L2] = '\0';

    return 1;
}

uint8_t fromBcd(uint8_t num) {
    return ((num >> 4) * 10) + (num & 0x0F);
}

int readDataPacket(uint8_t *sequenceNumber, uint8_t *packetData) {
    uint8_t packet[MAX_PAYLOAD_SIZE];

    int bytes = llread(packet);
    if (bytes < 0)
        return -1;
    if (bytes < BASE_CONTROL_PACKET_SIZE) {
        errorLog(__func__, "Packet too small to be data packet");
        return -1;
    }

    uint8_t controlField = packet[0];
    if (controlField != CF_DATA) {
        errorLog(__func__, "Control field mismatch (received %d, expected %d)", controlField, CF_DATA);
        return -1;
    }
    
    *sequenceNumber = fromBcd(packet[1]);
    
    uint16_t packetDataSize = (packet[2] << 8) + packet[3];
    if (packetDataSize > MAX_DATA_PACKET_PAYLOAD_SIZE) {
        errorLog(__func__, "Max data packet size exceeded");
        return -1;
    }

    memcpy(packetData, packet + 4, packetDataSize);
    
    return packetDataSize;
}

int writeControlPacket(uint8_t controlField, uint32_t filesize, const char *filename) {
    int filenameSize = strlen(filename);
    if (filenameSize > MAX_FILENAME_SIZE) {
        errorLog(__func__, "Max filename size exceeded");
        return -1;
    }

    uint8_t buf[BASE_CONTROL_PACKET_SIZE + MAX_FILENAME_SIZE];

    buf[0] = controlField;

    // TLV for file size
    buf[1] = TLV_FILESIZE;
    buf[2] = 4;
    buf[3] = (filesize >> 24) & 0xFF;
    buf[4] = (filesize >> 16) & 0xFF;
    buf[5] = (filesize >> 8) & 0xFF;
    buf[6] = (filesize) & 0xFF;

    // TLV for file name
    buf[7] = TLV_FILENAME;
    buf[8] = filenameSize;
    memcpy(buf + 9, filename, filenameSize);

    return llwrite(buf, BASE_CONTROL_PACKET_SIZE + filenameSize);
}

uint8_t toBcd(uint8_t num) {
    return ((num / 10) << 4) + (num % 10);
}

int writeDataPacket(uint8_t sequenceNumber, uint16_t packetDataSize, const uint8_t *packetData) {
    if (packetDataSize > MAX_DATA_PACKET_PAYLOAD_SIZE) {
        errorLog(__func__, "Max packet data size exceeded");
        return -1;
    }

    uint8_t buf[BASE_DATA_PACKET_SIZE + MAX_DATA_PACKET_PAYLOAD_SIZE];

    buf[0] = CF_DATA;
    buf[1] = toBcd(sequenceNumber);
    buf[2] = (packetDataSize >> 8) & 0xFF;
    buf[3] = (packetDataSize) & 0xFF;
    memcpy(buf + 4, packetData, packetDataSize);

    return llwrite(buf, BASE_DATA_PACKET_SIZE + packetDataSize);
}
