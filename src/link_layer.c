// Link layer protocol implementation

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "link_layer.h"
#include "serial_port.h"
#include "special_bytes.h"
#include "state.h"
#include "frame.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int alarmEnabled = FALSE;
int alarmCount = 0;

#define MAX_RETRIES     5

// Alarm function handler
void alarmHandler(int signal) {
    alarmEnabled = TRUE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}

unsigned char nextRecvState(
    unsigned char state,
    unsigned char byte,
    unsigned char cByte
) {
    printf(":%02x", byte);

    switch (state) {
      case STATE_START:
        if (byte == FLAG)
            state = STATE_FLAG_RCV;
        break;

      case STATE_FLAG_RCV:
        if (byte == A1)
            state = STATE_A_RCV;
        else if (byte != FLAG)
            state = STATE_START;
        break;

      case STATE_A_RCV:
        if (byte == cByte)
            state = STATE_C_RCV;
        else if (byte == FLAG)
            state = STATE_FLAG_RCV;
        else
            state = STATE_START;
        break;

      case STATE_C_RCV:
        if (byte == (A1 ^ cByte))
            state = STATE_BCC1_OK;
        else if (byte == FLAG)
            state = STATE_FLAG_RCV;
        else
            state = STATE_START;
        break;

      case STATE_BCC1_OK:
        if (byte == FLAG)
            state = STATE_STOP;
        else
            state = STATE_START;
        break;

      default:
        break;
    }

    return state;
}

unsigned char nextReadState(
    unsigned char state,
    unsigned char byte,
    unsigned char frameNumber,
    unsigned char *xor
) {
    printf(":%02x", byte);

    switch (state) {
      case STATE_START:
        if (byte == FLAG)
            state = STATE_FLAG_RCV;
        break;

      case STATE_FLAG_RCV:
        if (byte == A1)
            state = STATE_A_RCV;
        else if (byte != FLAG)
            state = STATE_START;
        break;

      case STATE_A_RCV:
        if ((frameNumber == 0 && byte == C(0)) || (frameNumber == 1 && byte == C(1)))
            state = STATE_C_RCV;
        else if (byte == FLAG)
            state = STATE_FLAG_RCV;
        else
            state = STATE_START;
        break;

      case STATE_C_RCV:
        if (byte == (A1 ^ (frameNumber == 0 ? C(0) : C(1))))
            state = STATE_BCC1_OK;
        else if (byte == FLAG)
            state = STATE_FLAG_RCV;
        else
            state = STATE_START;
        break;

      case STATE_BCC1_OK:
        if (byte == FLAG) {
            state = *xor == 0 ? STATE_STOP : STATE_BCC2_BAD;
        } else {
            *xor ^= byte;
        }
        break;

      default:
        break;
    }

    return state;
}

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
    if (role == LlTx) {
        (void)signal(SIGALRM, alarmHandler);

        unsigned char buf[6] = {FLAG, A1, SET, A1 ^ SET, FLAG, '\0'};
        unsigned char state = STATE_START;

        while (alarmCount < MAX_RETRIES) {
            
            if (!alarmEnabled) {
                int bytes = writeBytesSerialPort(buf, 5);
                if (bytes < 0) {
                    perror("Error writing serial port");
                    return -1;
                }
                printf("%d bytes written\n", bytes);
                state = STATE_START;

                alarm(3);
                alarmEnabled = TRUE;
            }

            // Wait until all bytes have been written to the serial port
            sleep(1);

            while (state != STATE_STOP && alarmEnabled) {
                unsigned char byte;
                int r = readByteSerialPort(&byte);
                if (r < 0){
                    perror("openRecv");
                    return -1;
                }

                if (r > 0) {
                    printf(":%02x\t", byte);

                    switch (state) {
                    case STATE_START:
                        if (byte == FLAG) 
                            state = STATE_FLAG_RCV;
                        break;
                    case STATE_FLAG_RCV:
                        if (byte == A1) 
                            state = STATE_A_RCV;
                        else if (byte != FLAG)
                            state = STATE_START;
                        break;
                    case STATE_A_RCV:
                        if (byte == UA) 
                            state = STATE_C_RCV;
                        else if (byte == FLAG)
                            state = STATE_FLAG_RCV;
                        else 
                            state = STATE_START;
                        break;
                    case STATE_C_RCV:
                        if (byte == (A1 ^ UA)) 
                            state = STATE_BCC1_OK;
                        else if (byte == FLAG)
                            state = STATE_FLAG_RCV;
                        else 
                            state = STATE_START;
                        break;
                    case STATE_BCC1_OK:
                        if (byte == FLAG) 
                            state = STATE_STOP;
                        else 
                            state = STATE_START;
                        break;
                    default:
                        break;
                    }
                    printf("next state: %d\n", state);
                }
            }

            printf("Try #%d\n", alarmCount);
        }

        alarm(0);
        alarmEnabled = FALSE;

        return 1;
    } else if (role == LlRx) {
        
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

        if (readSOrUFrame(A1, SET) < 0)
            return -1;
        if (writeSOrUFrame(A1, UA) < 0)
            return -1;
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    (void)signal(SIGALRM, alarmHandler);
    unsigned char bcc2 = 0;
    unsigned char msg[MAX_PAYLOAD_SIZE] = {FLAG, A1, C(0), A1 ^ C(0)};
    for (int i = 0; i < bufSize; i++){
        msg[4+i] = buf[i];
        bcc2 ^= buf[i];
    }
    msg[4+bufSize] = bcc2;
    msg[4+bufSize+1] = FLAG;

    unsigned char state = STATE_START;
    int status = REJ;

    while (alarmCount < MAX_RETRIES && status != ACC) {

        if (!alarmEnabled) {
            alarm(3);
            alarmEnabled = TRUE;

            int bytes = writeBytesSerialPort(msg, 40);
            if (bytes < 0) {
                perror("Error writing serial port");
                return -1;
            }
            printf("%d bytes written\n", bytes);

            // Wait until all bytes have been written to the serial port
            sleep(1);
            state = STATE_START;
        }

        while (state != STATE_STOP && alarmEnabled) {
            unsigned char byte;
            int r = readByteSerialPort(&byte);
            if (r < 0){
                perror("openRecv");
                return -1;
            }

            if (r > 0) {
                printf(":%02x\t", byte);

                switch (state) {
                case STATE_START:
                    if (byte == FLAG) 
                        state = STATE_FLAG_RCV;
                    break;
                case STATE_FLAG_RCV:
                    if (byte == A1) 
                        state = STATE_A_RCV;
                    else if (byte != FLAG)
                        state = STATE_START;
                    break;
                case STATE_A_RCV:
                    if (byte == RR1){
                        state = STATE_C_RCV;
                        status = ACC;
                    }
                    else if (byte == REJ1) {
                        state = STATE_C_RCV;
                        status = REJ;
                    }
                    else if (byte == FLAG)
                        state = STATE_FLAG_RCV;
                    else 
                        state = STATE_START;
                    break;
                case STATE_C_RCV:
                    if (status == ACC && byte == (A1 ^ RR1))
                        state = STATE_BCC1_OK;
                    else if (status == REJ && byte == (A1 ^ REJ1))
                        state = STATE_BCC1_OK;
                    else if (byte == FLAG)
                        state = STATE_FLAG_RCV;
                    else 
                        state = STATE_START;
                    break;
                case STATE_BCC1_OK:
                    if (byte == FLAG) 
                        state = STATE_STOP;
                    else 
                        state = STATE_START;
                    break;
                default:
                    break;
                }
                printf("next state: %d\n", state);
            }
        }

        printf("Try #%d\n", alarmCount);
        printf("Status %d\n", status);
    }

    alarm(0);
    alarmEnabled = FALSE;

    return 1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    unsigned char xor = 0;
    unsigned char state = STATE_START;

    int index = 0;
    while (state != STATE_STOP && state != STATE_BCC2_BAD) {
        unsigned char byte;
        int r = readByteSerialPort(&byte);
        if (r < 0) {
            perror("openRecv");
            return -1;
        }
        
        if (r == 1) {
            if (state == STATE_BCC1_OK)
                packet[index++] = byte;

            state = nextReadState(state, byte, 0, &xor);
            if (state == STATE_BCC2_BAD) {
                
            }
        }
    }

    int bytes = writeBytesSerialPort(
        (unsigned char[]) { FLAG, A1, REJ1, A1 ^ REJ1, FLAG },
        5
    );

    if (bytes < 0) {
        perror("llopen");
        return -1;
    }

    printf("%d bytes written\n", bytes);

    return 1;
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
