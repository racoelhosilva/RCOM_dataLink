// Application layer protocol implementation

#include <stdio.h>
#include <string.h>

#include "application_layer.h"
#include "link_layer.h"
#include "special_bytes.h"
#include "packet.h"
#include "debug.h"
#include "statistics.h"

uint32_t getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    rewind(file);
    return size;
}

int sendFile(FILE* file) {
    uint8_t sequenceNumber = 0;
    uint16_t payloadSize = 0;
    uint8_t buf[MAX_DATA_PACKET_PAYLOAD_SIZE];

    payloadSize = fread(&buf, 1, MAX_DATA_PACKET_PAYLOAD_SIZE, file);
    while (payloadSize > 0) {
        
        int r = writeDataPacket(sequenceNumber, payloadSize, buf);
        if (r < 0) {
            return -1;
        }
        if (r < payloadSize) {
            errorLog(__func__, "Written data packet smaller than payload size");
            return -1;
        }

        statistics.dataBytes += payloadSize;

        payloadSize = fread(&buf, 1, MAX_DATA_PACKET_PAYLOAD_SIZE, file);
        sequenceNumber = (sequenceNumber + 1) % 100;
    }
    return 1;
}

int receiveFile(FILE* file, uint32_t filesize) {
    uint8_t data[MAX_PAYLOAD_SIZE];
    int dataSize;
    uint8_t sequenceNumber = 0, sequenceNumberCheck = 0;
    uint32_t totalDataSize = 0;    

    while (totalDataSize < filesize) {
        dataSize = readDataPacket(&sequenceNumber, data);
        if (dataSize < 0)
            return -1;
        
        if (sequenceNumber != sequenceNumberCheck) {
            errorLog(__func__, "Unexpected packet sequence number (received %d, expected %d)", sequenceNumberCheck, sequenceNumber);
            return -1;
        }

        fwrite(data, 1, dataSize, file);
        totalDataSize += dataSize;
        statistics.dataBytes += dataSize;
        sequenceNumberCheck = (sequenceNumberCheck + 1) % 100;
    }
    return 1;
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {
    if (serialPort == NULL || role == NULL || filename == NULL) {
        errorLog(__func__, "Null pointer passed as parameter");
        return;
    }

    LinkLayer connectionParameters;
    strncpy(connectionParameters.serialPort, serialPort, 50);
    connectionParameters.role = strcmp(role, "tx") == 0 ? LlTx : LlRx;
    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    if (llopen(connectionParameters) < 0) {
        return;
    }

    if (connectionParameters.role == LlTx) {
        
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            errorLog(__func__, "Couldn't open file to transmit");
            return;
        }

        uint32_t size = getFileSize(file);

        if (writeControlPacket(CF_START, size, filename) < 0)
            return;

        if (sendFile(file) < 0) {
            return;
        }
        
        if (fclose(file)) {
            errorLog(__func__, "Couldn't close transmitted file");
            return;
        }

        if (writeControlPacket(CF_END, size, filename) < 0)
            return;

    } else {

        FILE *file = fopen(filename, "w");
        if (file == NULL) {
            errorLog(__func__, "Couldn't open file to receive");
            return;
        }

        uint8_t controlFieldCheck;
        uint32_t filesize1;
        char filename1[MAX_FILENAME_SIZE + 1];
        readControlPacket(&controlFieldCheck, &filesize1, filename1);

        if (controlFieldCheck != CF_START) {
            errorLog(__func__, "Invalid initial packet control field");
            return;
        }

        if (receiveFile(file, filesize1) < 0) {
            return;
        }

        uint32_t filesize2;
        char filename2[MAX_FILENAME_SIZE + 1];
        readControlPacket(&controlFieldCheck, &filesize2, filename2);

        if (controlFieldCheck != CF_END) {
            errorLog(__func__, "Invalid final packet control field");
            return;
        }

        if (filesize1 != filesize2 || strcmp(filename1, filename2) != 0) {
            errorLog(__func__, "Mismatch between start and end packet parameters");
            return;
        }

        uint32_t size = getFileSize(file);
        if (filesize1 != size) {
            errorLog(__func__, "File size mismatch");
            return;
        }

        if (fclose(file)){
            errorLog(__func__, "Couldn't close received file");
            return;
        }
    }
    
    llclose(1);
}
