#ifndef _FRAME_H_
#define _FRAME_H_

#include <stdint.h>

#include "alarm.h"

#define SU_FRAME_SIZE     5 // Minimum frame size of S or U frames
#define I_FRAME_BASE_SIZE 5 // Minimum frame size of I frame
#define MAX_BCC2_SIZE     2 // Maximum size of BCC 2

// Reads a S/U frame, checking the address field and extracting the control field.
// Returns 1 on success and a negative value otherwise.
int readSOrUFrame(uint8_t addressField, uint8_t *controlField, int timeout);

// Reads an I frame, checking the address field and extracting the frame number and data integrity.
// Returns 1 on success and a negative value on error, meaning:
//   -1 => Unknown error
//   -2 => Error in data integrity (BCC2 mismatch or data size exceeded)
//   -3 => Reception of SET frame
//   -4 => Reception of DISC frame
int readIFrame(uint8_t addressField, uint8_t *frameNumber, uint8_t *data);

// Writes any type of frame into the serial port.
// Returns 1 on success and a negative value otherwise. 
int writeFrame(const uint8_t *frame, int frameSize);

// Writes a Supervision or Unnumbered frame.
// Returns 1 on success and a negative value otherwise. 
int writeSOrUFrame(uint8_t addressField, uint8_t controlField);

// Writes an Information frame.
// Returns the number of bytes written.
int writeIFrame(uint8_t addressField, uint8_t frameNumber, const uint8_t* data, int dataSize);

#endif  // _FRAME_H_
