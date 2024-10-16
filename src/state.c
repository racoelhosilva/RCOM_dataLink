#include "special_bytes.h"
#include "state.h"

State nextKnownSOrUFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t controlField)
{
    switch (state) {
      case STATE_START:
        if (byte == FLAG)
            state = STATE_FLAG_RCV;
        break;

      case STATE_FLAG_RCV:
        if (byte == addressField)
            state = STATE_A_RCV;
        else if (byte != FLAG)
            state = STATE_START;
        break;

      case STATE_A_RCV:
        if (byte == controlField)
            state = STATE_C_RCV;
        else if (byte == FLAG)
            state = STATE_FLAG_RCV;
        else
            state = STATE_START;
        break;

      case STATE_C_RCV:
        if (byte == (addressField ^ controlField))
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

int isValidSOrUFrameControl(uint8_t byte) {
    return byte == SET || byte == UA || byte == RR(0) || byte == RR(1) || byte == REJ0 || byte == REJ1 || byte == DISC;
}

State nextSOrUFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t *controlField)
{
    switch (state) {
      case STATE_START:
        if (byte == FLAG)
            state = STATE_FLAG_RCV;
        break;

      case STATE_FLAG_RCV:
        if (byte == addressField)
            state = STATE_A_RCV;
        else if (byte != FLAG)
            state = STATE_START;
        break;

      case STATE_A_RCV:
        if (isValidSOrUFrameControl(byte)) {
            *controlField = byte;
            state = STATE_C_RCV;
        } else if (byte == FLAG) {
            state = STATE_FLAG_RCV;
        } else {
            state = STATE_START;
        }
        break;

      case STATE_C_RCV:
        if (byte == (addressField ^ *controlField))
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

State nextIFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t *frameNumber, uint8_t *xor) {
    switch (state) {
      case STATE_START:
        if (byte == FLAG)
            state = STATE_FLAG_RCV;
        break;

      case STATE_FLAG_RCV:
        if (byte == addressField)
            state = STATE_A_RCV;
        else if (byte != FLAG)
            state = STATE_START;
        break;

      case STATE_A_RCV:
        if (byte == C(0)) {
            *frameNumber = 0;
            state = STATE_C_RCV;
        } else if (byte == C(1)) {
            *frameNumber = 1;
            state = STATE_C_RCV;
        } else if (byte == FLAG) {
            state = STATE_FLAG_RCV;
        } else {
            state = STATE_START;
        }
        break;

      case STATE_C_RCV:
        if (byte == (addressField ^ C(*frameNumber)))
            state = STATE_BCC1_OK;
        else if (byte == FLAG)
            state = STATE_FLAG_RCV;
        else
            state = STATE_START;
        break;

      case STATE_BCC1_OK:
        state = STATE_DATA;
        *xor = byte;
        break;

      case STATE_DATA:
        if (byte == FLAG)
            state = *xor == 0 ? STATE_STOP : STATE_BCC2_BAD;
        else
            *xor ^= byte;
        break;

      default:
        break;
    }

    return state;
}
