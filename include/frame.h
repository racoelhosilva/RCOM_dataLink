#ifndef _FRAME_H_
#define _FRAME_H_

#include <stdint.h>

#define FRAME_BASE_SIZE 5

int readSOrUFrame(uint8_t addressField, uint8_t controlField);


int writeSOrUFrame(uint8_t addressField, uint8_t controlField);

#endif  // _FRAME_H_
