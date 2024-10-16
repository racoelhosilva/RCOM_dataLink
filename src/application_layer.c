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
        unsigned char message[] = "Mic Test 1,2,1 is this on?";
        message[26] = 'n';
        int r = llwrite(message, 27);
        if (r < 0){
            perror("Fail llwrite");
            return;
        }

    } else {
        unsigned char buf[MAX_PAYLOAD_SIZE] = {0};
        int bytes = llread(buf);
        if (bytes < 0)
            return;

        printf(":");
        for (int i = 0; i < bytes + 2; i++)
            printf("%02x:", buf[i]);
    }
}
