// Application layer protocol implementation

#include <stdio.h>
#include <string.h>

#include "application_layer.h"
#include "link_layer.h"
#include "packet.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer connectionParameters;
    strncpy(connectionParameters.serialPort, serialPort, 50);
    connectionParameters.role = strcmp(role, "tx") == 0 ? LlTx : LlRx;
    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    if (llopen(connectionParameters) < 0) {
        perror("applicationLayer");
        return;
    }

    if (connectionParameters.role == LlTx) {
        /*for (int i = 0; i < 5; i++) {
            unsigned char message[35];
            sprintf((char *)message, "Mic Test %d,%d,%d is this on ~~?", i + 1, i + 2, i + 1);

            printf("Message to transmit: %s\n", (char *)message);
            printf("Message length: %d\n", (int)sizeof(message));

            int r = llwrite(message, sizeof(message));
            if (r < 0){
                perror("Fail llwrite");
                return;
            }
        }*/

        writeControlPacket(1, 633, "random.txt");
        writeDataPacket(0, 211, (uint8_t *)"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
        writeDataPacket(0, 211, (uint8_t *)"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
        writeDataPacket(0, 211, (uint8_t *)"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
        writeControlPacket(3, 633, "random.txt");

    } else {
        /*unsigned char buf[MAX_PAYLOAD_SIZE] = {0};
        // TODO: How to know when a packet is the last?
        for (int i = 0; i < 5; i++) {
            int bytes = llread(buf);
            if (bytes < 0)
                return;

            printf("Received message: %s\n", (char *)buf);
            printf("Message length: %d\n", bytes);
        }*/

        uint8_t controlFieldCheck;
        uint32_t filesize1;
        char filename1[MAX_FILENAME_SIZE];
        readControlPacket(&controlFieldCheck, &filesize1, filename1);

        printf("Filename: %s\n", filename1);
        printf("Filename size: %lu\n", strlen(filename1));
        printf("File size: %d\n", filesize1);

        uint8_t data[MAX_DATA_PACKET_PAYLOAD_SIZE];
        int dataSize;
        uint8_t sequenceNumber;

        uint32_t totalDataSize = 0;    
        while (totalDataSize < filesize1) {
            dataSize = readDataPacket(&controlFieldCheck, &sequenceNumber, data);

            printf("Data received: %s\n", (char *)data);
            printf("Data size: %d\n", dataSize);
            
            totalDataSize += dataSize;
        }

        uint32_t filesize2;
        char filename2[MAX_FILENAME_SIZE];
        readControlPacket(&controlFieldCheck, &filesize2, filename2);

        printf("Filename: %s\n", filename2);
        printf("Filename size: %lu\n", strlen(filename2));
        printf("File size: %d\n", filesize2);

    }

    llclose(0);
}
