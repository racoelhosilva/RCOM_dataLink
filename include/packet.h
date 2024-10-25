#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdint.h>

#include "link_layer.h"

#define BASE_CONTROL_PACKET_SIZE     9                                              // Control packet base size
#define BASE_DATA_PACKET_SIZE        4                                              // Data packet base size
#define MAX_FILENAME_SIZE            (MAX_PAYLOAD_SIZE - BASE_CONTROL_PACKET_SIZE)  // Control packet max filename length
#define MAX_DATA_PACKET_PAYLOAD_SIZE (MAX_PAYLOAD_SIZE - BASE_DATA_PACKET_SIZE)     // Data packet max payload size

// Reads a control packet from the link layer.
// Returns 1 on success and a negative value otherwise.
int readControlPacket(uint8_t* controlField, uint32_t *filesize, char *filename);

// Reads a data packet from the link layer.
// Returns 1 on success and a negative value otherwise.
int readDataPacket(uint8_t *sequenceNumber, uint8_t *packetData);

// Writes a control packet to the link layer.
// Returns 1 on success and a negative value otherwise.
int writeControlPacket(uint8_t controlField, uint32_t filesize, const char *filename);

// Writes a data packet to the link layer.
// Returns 1 on success and a negative value otherwise.
int writeDataPacket(uint8_t sequenceNumber, uint16_t packetDataSize, const uint8_t *packetData);

#endif  // _PACKET_H_
