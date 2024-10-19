#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>

typedef enum {
    STATE_START,
    STATE_FLAG_RCV,
    STATE_A_RCV,
    STATE_C_RCV,
    STATE_BCC1_OK,
    STATE_DATA,
    STATE_DATA_ESC,
    STATE_DATA_STUFF,
    STATE_DATA_WRT_STUFF,
    STATE_DATA_ESC_WRT_STUFF,
    STATE_STOP,
    STATE_BCC2_BAD,
} State;

int isDataState(State state);

State nextKnownSOrUFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t controlField);
State nextSOrUFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t *controlField);

State nextIFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t *frameNumber, uint8_t xor);

#endif  // _STATE_H_
