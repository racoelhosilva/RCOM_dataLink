#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <sys/time.h>

#include "link_layer.h"

#ifndef STATISTICS
#define STATISTICS 0
#endif

typedef struct {
    LinkLayerRole role;
    unsigned int baudrate;
    struct timeval start;
    struct timeval end;
    unsigned int dataBytes;
    unsigned int totalBytes;
    unsigned int totalFrames;
    unsigned int badFrames;
    unsigned int totalRej;
    unsigned int totalTimeouts;
} Statistics;

extern Statistics statistics;

void initStatistics(const LinkLayer *connectionParameters);
void printStatistics();
int storeStatistics();

#endif  //_STATISTICS_H_