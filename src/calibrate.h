#ifndef __CALIBRATE_H__
#define __CALIBRATE_H__

#include <ptime.h>

#define WORK(n)                                                                \
    do {                                                                       \
        int a = 1234;                                                          \
        int b = 5679;                                                          \
        for (int j = 0; j < n; j++)                                            \
            a = (b * a);                                                       \
    } while (0)

#define CALIBRATE_FILE "iterations.txt"

/* opens file iterations.txt */
long calibrate();
/* reads from the environment variable PTASK_CALIBRATE_ITER */
long calibrate_env();

/* work for a certain amount of time (step of ~ 1 milliseconds) */
void work_for(ptime delay, int unit);

#endif
