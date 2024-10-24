#ifndef _SPECIAL_BYTES_H_
#define _SPECIAL_BYTES_H_

#define FLAG 0x7E  // Flag byte

#define A1   0x03  // Address field for commands from the transmitter or replies from the receiver
#define A2   0x01  // Address field for commands from the receiver or replies from the transmitter

#define SET    0x03          // SET frame control field
#define UA     0x07          // UA frame control field
#define RR(n)  (0xAA + (n))  // RR<n> frame control field
#define REJ(n) (0x54 + (n))  // REJ<n> frame control field
#define DISC   0x0B          // DISC frame control field

#define C(n) (0x80 * ((n) & 1))  // Information frame n control field

#define ESC       0x7D  // Escape octet
#define ESC2_FLAG 0x5E  // Escaped flag 2nd byte
#define ESC2_ESC  0x5D  // Escaped ESC 2nd byte

#define TLV_FILESIZE    0
#define TLV_FILENAME    1

#define CF_START    1
#define CF_DATA     2
#define CF_END      3

#endif  // _SPECIAL_BYTES_H_