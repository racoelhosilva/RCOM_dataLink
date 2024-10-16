// Application layer protocol implementation

#include <stdio.h>
#include <string.h>

#include "application_layer.h"
#include "link_layer.h"

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

    if (strcmp(role, "tx") == 0) {
        for (int i = 0; i < 5; i++) {
            unsigned char message[27];
            sprintf((char *)message, "Mic Test %d,%d,%d is this on?", i + 1, i + 2, i + 1);

            printf("Message to transmit: %s\n", (char *)message);
            printf("Message length: %d\n", (int)sizeof(message));

            int r = llwrite(message, sizeof(message));
            if (r < 0){
                perror("Fail llwrite");
                return;
            }
        }

    } else {
        unsigned char buf[MAX_PAYLOAD_SIZE] = {0};
        
        // TODO: How to know when a packet is the last?
        for (int i = 0; i < 5; i++) {
            int bytes = llread(buf);
            if (bytes < 0)
                return;

            printf("Received message: %s\n", (char *)buf);
            printf("Message length: %d\n", bytes);
        }
    }
}
