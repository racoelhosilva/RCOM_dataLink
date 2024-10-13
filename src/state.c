#include "special_bytes.h"
#include "state.h"

State nextSOrUFrameState(State state, uint8_t byte, uint8_t addressField, uint8_t controlField)
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