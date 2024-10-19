#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdint.h>

#include "link_layer.h"

#define MAX_FILENAME_SIZE (((MAX_PAYLOAD_SIZE - 4) / 2) - 5)
#define MAX_DATA_PACKET_SIZE (((MAX_PAYLOAD_SIZE - 1) / 2) - 3)


int readControlPacket();

int writeControlPacket(uint8_t controlField, uint32_t filesize, uint8_t filenameSize, char *filename);
int writeDataPacket(uint8_t sequenceNumber, uint16_t packetDataSize, uint8_t *packetData);

#endif  // _PACKET_H_
