#ifndef __CALIBRATE_H__
#define __CALIBRATE_H__

#define WORK(n) do { int a = 1234; int b = 5679;   \
        for (int j=0; j<n; j++) a=(b*a);           \
    } while (0)

#define CALIBRATE_FILE "iterations.txt"

long calibrate();

#endif
