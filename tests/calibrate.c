#include "calibrate.h"
#include <stdio.h>
#include <stdlib.h>
#include <tstat.h>

static long iter_milli = 0;

long read_calibrate_file()
{
    FILE *f = fopen(CALIBRATE_FILE, "r");
    if (f == NULL) {
        perror("Cannot open calibration file");
        fprintf(stderr,"Run tests/wcet_calibration to generate the calibration file\n");
        exit(-1);
    }
    fscanf(f, "%ld", &iter_milli);
    fclose(f);
    return iter_milli;
}

long read_calibrate_env()
{
    char *var = getenv(PTASK_CALIBRATE_ITER);
    if (!var) {
        perror("Cannot find the " PTASK_CALIBRATE_ITER " environment variable\n");
        fprintf(stderr, "tests/wcet_calibration to generate the calibration script");
        exit(-1);
    }

    iter_milli = strtol(var, NULL, 10);
    if (iter_milli == 0) 
        fprintf(stderr, "Warning : Iterations = 0\n");
    
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
