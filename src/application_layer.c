// Application layer protocol implementation

#include <stdio.h>
#include <string.h>

#include "application_layer.h"
#include "link_layer.h"
#include "packet.h"

uint32_t getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    rewind(file);
    return size;
}

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
        
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            perror("applicationLayer");
            return;
        }

        uint32_t size = getFileSize(file);

        writeControlPacket(1, size, filename);

        uint8_t sequenceNumber = 0;
        uint16_t payloadSize = 0;
        uint8_t buf[MAX_DATA_PACKET_PAYLOAD_SIZE];

        payloadSize = fread(&buf, 1, MAX_DATA_PACKET_PAYLOAD_SIZE, file);
        while (payloadSize > 0) {
            
            int r = writeDataPacket(sequenceNumber, payloadSize, buf);
            if (r < 0) {
                break;
            }
            payloadSize = fread(&buf, 1, MAX_DATA_PACKET_PAYLOAD_SIZE, file);
            sequenceNumber = (sequenceNumber + 1) % 100;
        }
        
        if (fclose(file)) {
            perror("applicationLayer");
            return;
        }
        writeControlPacket(3, size, filename);

    } else {

        FILE *file = fopen(filename, "w");
        if (file == NULL) {
            perror("applicationLayer");
            return;
        }

        uint8_t controlFieldCheck;
        uint32_t filesize1;
        char filename1[MAX_FILENAME_SIZE + 1];
        readControlPacket(&controlFieldCheck, &filesize1, filename1);

        if (controlFieldCheck != 1) {
            perror("applicationLayer");
            return;
        }

        uint8_t data[MAX_DATA_PACKET_PAYLOAD_SIZE];
        int dataSize;
        uint8_t sequenceNumber, sequenceNumberCheck = 0;
        uint32_t totalDataSize = 0;    

        while (totalDataSize < filesize1) {
            dataSize = readDataPacket(&controlFieldCheck, &sequenceNumber, data);
            if (dataSize < 0 || controlFieldCheck != 2 || sequenceNumber != sequenceNumberCheck)
                return;

            fwrite(data, 1, dataSize, file);
            totalDataSize += dataSize;
            sequenceNumberCheck = (sequenceNumberCheck + 1) % 100;
        }

        uint32_t filesize2;
        char filename2[MAX_FILENAME_SIZE + 1];
        readControlPacket(&controlFieldCheck, &filesize2, filename2);

        if (controlFieldCheck != 3 || filesize1 != filesize2 || strcmp(filename1, filename2) != 0) {
            perror("applicationLayer");
            return;
        }

        fclose(file);
    }

    llclose(0);
}
