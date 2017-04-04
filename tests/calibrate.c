#include "calibrate.h"
#include <stdio.h>

long calibrate()
{
    FILE *f = fopen(CALIBRATE_FILE, "r");
    long v;
    fscanf(f, "%ld", &v);
    fclose(f);
    return v;
}
