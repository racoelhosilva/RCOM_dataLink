#include "statistics.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"

Statistics statistics;

void initStatistics(const LinkLayer *connectionParameters) {
    statistics.role = connectionParameters->role;
    statistics.baudrate = connectionParameters->baudRate;
    statistics.dataBytes = 0;
    statistics.totalBytes = 0;
    statistics.totalFrames = 0;
    statistics.badFrames = 0;
    statistics.totalRej = 0;
    statistics.totalTimeouts = 0;
}

void printStatistics() {
    double totalTime = (statistics.end.tv_sec - statistics.start.tv_sec) * 1e9 + (statistics.end.tv_nsec - statistics.start.tv_nsec);
    double measuredBaudrate = statistics.dataBytes * 8 * 1e9 / totalTime;
    double efficiency = measuredBaudrate / statistics.baudrate;

    printf("\n");
    printf("************ STATISTICS ************\n");
    printf("\n");
    printf("Communication time: %f s\n", totalTime / 1e9);
    printf("Serial port baudrate: %d\n", statistics.baudrate);
    printf("Payload size: %d\n", MAX_PAYLOAD_SIZE);
    printf("Measured baudrate: %f\n", measuredBaudrate);
    printf("Efficiency: %f\n", efficiency);
    printf("\n");

    switch (statistics.role) {
      case LlTx:
        printf("Total bytes transmitted: %d\n", statistics.totalBytes);
        printf("Data bytes transmitted: %d\n", statistics.dataBytes);
        printf("Total frames transmitted: %d\n", statistics.totalFrames);
        printf("Total negative acknowledgements: %d\n", statistics.totalRej);
        printf("Total timeouts: %d\n", statistics.totalTimeouts);
        printf("Frame error ratio: %f\n", (double)(statistics.totalRej + statistics.totalTimeouts) / statistics.totalFrames);
        break;

      case LlRx:
        printf("Total bytes received: %d\n", statistics.totalBytes);
        printf("Data bytes received: %d\n", statistics.dataBytes);
        printf("Total frames received: %d\n", statistics.totalFrames);
        printf("Good frames detected: %d\n", statistics.totalFrames - statistics.badFrames);
        printf("Bad frames detected: %d\n", statistics.badFrames);
        printf("Frame error ratio: %f\n", (double)statistics.badFrames / statistics.totalFrames);
        break;
    }

    printf("\n");
    printf("************************************\n");
}

int storeStatistics() {
    FILE *file;
    double totalTime = (statistics.end.tv_sec - statistics.start.tv_sec) * 1e9 + (statistics.end.tv_nsec - statistics.start.tv_nsec);
    double measuredBaudrate = statistics.dataBytes * 8 * 1e9 / totalTime;
    double efficiency = measuredBaudrate / statistics.baudrate;

    if (statistics.role == LlTx) {
        const char *filename = "stats-tx.csv";
        if (access(filename, F_OK) == 0) {
            file = fopen(filename, "a");
            if (file == NULL) {
                errorLog(__func__, "Couldn't open statistics spreadsheet");
                return -1;
            }

        } else {
            file = fopen(filename, "w");
            if (file == NULL) {
                errorLog(__func__, "Couldn't create statistics spreadsheet");
                return -1;
            }
        
            fprintf(file, "Bit error ratio,Propagation time,Baudrate,Payload size,Communication time (s),Measured baudrate,Efficiency,Total bytes,Data bytes,Total frames,NACK's,Timeouts,Frame error ratio\n");
        }

        fprintf(file, ",,%d,%d,%f,%f,%f,%d,%d,%d,%d,%d,%f\n",
            statistics.baudrate,
            MAX_PAYLOAD_SIZE,
            totalTime / 1e9,
            measuredBaudrate,
            efficiency,
            statistics.totalBytes,
            statistics.dataBytes,
            statistics.totalFrames,
            statistics.totalRej,
            statistics.totalTimeouts,
            (double)(statistics.totalRej + statistics.totalTimeouts) / statistics.totalFrames
        );
        
    } else {
        const char *filename = "stats-rx.csv";
        if (access(filename, F_OK) == 0) {
            file = fopen(filename, "a");
            if (file == NULL) {
                errorLog(__func__, "Couldn't open statistics spreadsheet");
                return -1;
            }

        } else {
            file = fopen(filename, "w");
            if (file == NULL) {
                errorLog(__func__, "Couldn't create statistics spreadsheet");
                return -1;
            }

            fprintf(file, "Bit error ratio,Propagation time,Baudrate,Payload size,Communication time (s),Measured baudrate,Efficiency,Total bytes,Data bytes,Total frames,Good frames,Bad frames,Frame error ratio\n");
        }

        fprintf(file, ",,%d,%d,%f,%f,%f,%d,%d,%d,%d,%d,%f\n",
            statistics.baudrate,
            MAX_PAYLOAD_SIZE,
            totalTime / 1e9,
            measuredBaudrate,
            efficiency,
            statistics.totalBytes,
            statistics.dataBytes,
            statistics.totalFrames,
            statistics.totalFrames - statistics.badFrames,
            statistics.badFrames,
            (double)statistics.badFrames / statistics.totalFrames
        );
    }

    return 1;
}
