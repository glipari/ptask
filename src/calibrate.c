#include "calibrate.h"
#include <stdio.h>
#include <stdlib.h>
#include <tstat.h>

static long iter_milli = 0;

long calibrate() {

    FILE *f = fopen(CALIBRATE_FILE, "r");
    if (f == NULL) {
        perror("Cannot open calibration file");
        printf("Run tests/wcet_calibration to generate the calibration file\n");
        exit(-1);
    }
    fscanf(f, "%ld", &iter_milli);
    fclose(f);
    return iter_milli;
}

void work_for(ptime delay, int unit) {
    tspec t = tstat_getexec(); // the start value
    ptime delta = 0;
    while (delay > delta) {
        WORK(iter_milli);
        tspec d = tstat_getexec();
        tspec diff = tspec_sub(&d, &t);
        delta = tspec_to(&diff, unit);
    }
}
