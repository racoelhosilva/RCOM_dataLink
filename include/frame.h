#ifndef _FRAME_H_
#define _FRAME_H_

#include <stdint.h>

#include "alarm.h"

#define SU_FRAME_SIZE     5
#define I_FRAME_BASE_SIZE 5
#define MAX_BCC2_SIZE     2

int readSOrUFrame(uint8_t addressField, uint8_t *controlField, int timeout);
int readIFrame(uint8_t addressField, uint8_t *frameNumber, uint8_t *data);

int writeSOrUFrame(uint8_t addressField, uint8_t controlField);
int writeIFrame(uint8_t addressField, uint8_t frameNumber, const uint8_t* data, int dataSize);

#endif  // _FRAME_H_
