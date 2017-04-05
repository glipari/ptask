#include "calibrate.h"
#include <stdio.h>
#include <stdlib.h>

long calibrate()
{
    long v;
    FILE *f = fopen(CALIBRATE_FILE, "r");
    if (f == NULL) {
        perror("Cannot open calibration file");
        printf("Run tests/wcet_calibration to generate the calibration file\n");
        exit(-1);
    }
    fscanf(f, "%ld", &v);
    fclose(f);
    return v;
}
