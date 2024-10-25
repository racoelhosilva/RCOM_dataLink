#include "statistics.h"

#include <stdlib.h>
#include <stdio.h>

Statistics statistics;

void initStatistics() {
    gettimeofday(&statistics.start, NULL);
    statistics.dataBytes = 0;
    statistics.totalBytes = 0;
    statistics.totalFrames = 0;
    statistics.badFrames = 0;
    statistics.totalRej = 0;
    statistics.totalTimeouts = 0;
}

void printStatistics(const LinkLayer *connectionParameters) {
    double totalTime = (statistics.end.tv_sec - statistics.start.tv_sec) * 1e6 + (statistics.end.tv_usec + statistics.start.tv_usec);
    double measuredBaudrate = statistics.dataBytes * 8 * 1e6 / totalTime;
    double efficiency = measuredBaudrate / connectionParameters->baudRate;

    printf("\n");
    printf("***** STATISTICS *****\n");
    printf("\n");
    printf("Communication time: %f s\n", totalTime / 1e6);
    printf("Serial port baudrate: %d\n", connectionParameters->baudRate);
    printf("Measured bitrate: %f\n", measuredBaudrate);
    printf("Efficiency: %f\n", efficiency);
    printf("\n");

    switch (connectionParameters->role) {
      case LlTx:
        printf("Total bytes transmitted: %d\n", statistics.totalBytes);
        printf("Data bytes transmitted: %d\n", statistics.dataBytes);
        printf("Total negative acknowledgements: %d\n", statistics.totalRej);
        printf("Total timeouts: %d\n", statistics.totalTimeouts);
        break;

      case LlRx:
        printf("Total bytes received: %d\n", statistics.totalBytes);
        printf("Data bytes received: %d\n", statistics.dataBytes);
        printf("Total frames received: %d\n", statistics.totalFrames);
        printf("Bad frames detected: %d\n", statistics.badFrames);
        break;
    }

    printf("\n");
    printf("**********************\n");
}