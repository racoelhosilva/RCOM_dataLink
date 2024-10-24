#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <sys/time.h>

#include "link_layer.h"

typedef struct {
    struct timeval start;
    struct timeval end;
    unsigned int dataBytes;
    unsigned int totalBytes;
    unsigned int totalFrames;
    unsigned int badFrames;
    unsigned int totalRetries;
    unsigned int maxRetries;
} Statistics;

extern Statistics statistics;

void initStatistics();
void printStatistics(const LinkLayer *connectionParameters);

#endif  //_STATISTICS_H_