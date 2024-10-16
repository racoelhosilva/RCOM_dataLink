// Link layer protocol implementation

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "link_layer.h"
#include "serial_port.h"
#include "special_bytes.h"
#include "state.h"
#include "frame.h"
#include "alarm.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int maxTries;
int timeout;
uint8_t frameNumber;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    const char* serialPort = connectionParameters.serialPort;
    int baudRate = connectionParameters.baudRate;
    if (openSerialPort(serialPort, baudRate) < 0)
        return -1;

    LinkLayerRole role = connectionParameters.role;
    maxTries = connectionParameters.nRetransmissions;
    timeout = connectionParameters.timeout;
    frameNumber = 0;

    if (role == LlTx) {
        configAlarm();

        for (int nTries = 0; nTries < maxTries; nTries++) {
            printf("Try #%d\n", alarmStatus.count);

            if (writeSOrUFrame(A1, SET) < 0)
                return -1;
            resetAlarm(timeout);

            // Wait until all bytes have been written to the serial port
            // sleep(1);  // TODO: Is this to be removed?

            int r = readKnownSOrUFrameTimeout(A1, UA);
            if (r != 0) {
                stopAlarm();
                return r;
            }

            // while (state != STATE_STOP && alarmStatus.enabled) {
                // unsigned char byte;
                // int r = readByteSerialPort(&byte);
                // if (r < 0){
                //     perror("openRecv");
                //     return -1;
                // }

                // if (r > 0) {
                //     printf(":%02x\t", byte);

                //     switch (state) {
                //     case STATE_START:
                //         if (byte == FLAG) 
                //             state = STATE_FLAG_RCV;
                //         break;
                //     case STATE_FLAG_RCV:
                //         if (byte == A1) 
                //             state = STATE_A_RCV;
                //         else if (byte != FLAG)
                //             state = STATE_START;
                //         break;
                //     case STATE_A_RCV:
                //         if (byte == UA) 
                //             state = STATE_C_RCV;
                //         else if (byte == FLAG)
                //             state = STATE_FLAG_RCV;
                //         else 
                //             state = STATE_START;
                //         break;
                //     case STATE_C_RCV:
                //         if (byte == (A1 ^ UA)) 
                //             state = STATE_BCC1_OK;
                //         else if (byte == FLAG)
                //             state = STATE_FLAG_RCV;
                //         else 
                //             state = STATE_START;
                //         break;
                //     case STATE_BCC1_OK:
                //         if (byte == FLAG) 
                //             state = STATE_STOP;
                //         else 
                //             state = STATE_START;
                //         break;
                //     default:
                //         break;
                //     }
                //     printf("next state: %d\n", state);
                // }
            // }
        }

        stopAlarm();
        perror("llopen");
        return -1;

    } else {
        
        // unsigned char state = STATE_START;
        // unsigned char (*nextState)(unsigned char, unsigned char, unsigned char)
        //     = nextRecvState;

        // while (state != STATE_STOP) {
        //     unsigned char byte;
        //     int r = readByteSerialPort(&byte);
        //     if (r < 0) {
        //         perror("openRecv");
        //         return -1;
        //     }
            
        //     if (r == 1) {
        //         state = nextState(state, byte, SET);
        //     }
        // }

        // int bytes = writeBytesSerialPort(
        //     (unsigned char[]) { FLAG, A, UA, A ^ UA, FLAG },
        //     5
        // );

        // if (bytes < 0) {
        //     perror("llopen");
        //     return -1;
        // }

        // printf("%d bytes written\n", bytes);

        if (readKnownSOrUFrame(A1, SET) < 0)
            return -1;
        if (writeSOrUFrame(A1, UA) < 0)
            return -1;

        // sleep(1);  // TODO: Remove this?
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    // configAlarm();

    // unsigned char bcc2 = 0;
    // unsigned char msg[MAX_PAYLOAD_SIZE] = {FLAG, A1, C(0), A1 ^ C(0)};
    // for (int i = 0; i < bufSize; i++){
    //     msg[4+i] = buf[i];
    //     bcc2 ^= buf[i];
    // }
    // msg[4+bufSize] = bcc2;
    // msg[4+bufSize+1] = FLAG;

    // unsigned char state = STATE_START;
    // int status = REJ;

    // while (alarmStatus.count < maxTries && status != ACC) {

    //     if (!alarmStatus.enabled) {
    //         alarm(3);
    //         alarmStatus.enabled = TRUE;

    //         int bytes = writeBytesSerialPort(msg, 40);
    //         if (bytes < 0) {
    //             perror("Error writing serial port");
    //             return -1;
    //         }
    //         printf("%d bytes written\n", bytes);

    //         // Wait until all bytes have been written to the serial port
    //         sleep(1);
    //         state = STATE_START;
    //     }

    //     while (state != STATE_STOP && alarmStatus.enabled) {
    //         unsigned char byte;
    //         int r = readByteSerialPort(&byte);
    //         if (r < 0){
    //             perror("openRecv");
    //             return -1;
    //         }

    //         if (r > 0) {
    //             printf(":%02x\t", byte);

    //             switch (state) {
    //             case STATE_START:
    //                 if (byte == FLAG) 
    //                     state = STATE_FLAG_RCV;
    //                 break;
    //             case STATE_FLAG_RCV:
    //                 if (byte == A1) 
    //                     state = STATE_A_RCV;
    //                 else if (byte != FLAG)
    //                     state = STATE_START;
    //                 break;
    //             case STATE_A_RCV:
    //                 if (byte == RR1){
    //                     state = STATE_C_RCV;
    //                     status = ACC;
    //                 }
    //                 else if (byte == REJ1) {
    //                     state = STATE_C_RCV;
    //                     status = REJ;
    //                 }
    //                 else if (byte == FLAG)
    //                     state = STATE_FLAG_RCV;
    //                 else 
    //                     state = STATE_START;
    //                 break;
    //             case STATE_C_RCV:
    //                 if (status == ACC && byte == (A1 ^ RR1))
    //                     state = STATE_BCC1_OK;
    //                 else if (status == REJ && byte == (A1 ^ REJ1))
    //                     state = STATE_BCC1_OK;
    //                 else if (byte == FLAG)
    //                     state = STATE_FLAG_RCV;
    //                 else 
    //                     state = STATE_START;
    //                 break;
    //             case STATE_BCC1_OK:
    //                 if (byte == FLAG) 
    //                     state = STATE_STOP;
    //                 else 
    //                     state = STATE_START;
    //                 break;
    //             default:
    //                 break;
    //             }
    //             printf("next state: %d\n", state);
    //         }
    //     }

    //     printf("Try #%d\n", alarmStatus.count);
    //     printf("Status %d\n", status);
    // }

    // alarm(0);
    // alarmStatus.enabled = FALSE;

    // return 1;

    
    configAlarm();

    for (int nTries = 0; nTries < maxTries; nTries++) {
        printf("Try #%d\n", alarmStatus.count);

        int bytes = writeIFrame(A1, frameNumber, buf, bufSize);
        if (bytes < 0)
            return -1;

        resetAlarm(timeout);

        // Wait until all bytes have been written to the serial port
        // sleep(1);  // TODO: Is this to be removed?

        uint8_t controlField = 0;

        int r;
        if ((r = readSOrUFrameTimeout(A1, &controlField)) < 0)
            return -1;
        if (r == 0 || (controlField != RR(0) && controlField != RR(1)))
            continue;

        uint8_t newFrameNumber = controlField == RR(0) ? 0 : 1;
        if (newFrameNumber != frameNumber) {
            frameNumber = newFrameNumber;
            return bytes;
        }
    }

    stopAlarm();
    perror("llwrite");
    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // unsigned char xor = 0;
    // unsigned char state = STATE_START;

    // int index = 0;
    // while (state != STATE_STOP && state != STATE_BCC2_BAD) {
    //     unsigned char byte;
    //     int r = readByteSerialPort(&byte);
    //     if (r < 0) {
    //         perror("openRecv");
    //         return -1;
    //     }
        
    //     if (r == 1) {
    //         if (state == STATE_BCC1_OK)
    //             packet[index++] = byte;

    //         state = nextReadState(state, byte, 0, &xor);
    //         if (state == STATE_BCC2_BAD) {
                
    //         }
    //     }
    // }

    // int bytes = writeBytesSerialPort(
    //     (unsigned char[]) { FLAG, A1, REJ1, A1 ^ REJ1, FLAG },
    //     5
    // );

    // if (bytes < 0) {
    //     perror("llopen");
    //     return -1;
    // }

    // printf("%d bytes written\n", bytes);

    int r;
    uint8_t readFrameNumber;

    for (int nTries = 0; nTries < maxTries; nTries++) {
        r = readIFrame(A1, &readFrameNumber, packet);
        if (r > 0) {
            if (readFrameNumber == frameNumber)
                frameNumber = !frameNumber;

            if (writeSOrUFrame(A1, RR(1)) > 0)
                return r;
        }
    }

    perror("llread");
    return -1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    int clstat = closeSerialPort();
    return clstat;
}
