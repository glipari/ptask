#include "pbarrier.h"
#include "pmutex.h"
#include <stdio.h>

void pbarrier_init(pbarrier_t *pb, int nth) {
    pthread_mutex_init(&pb->m, 0);
    pthread_cond_init(&pb->c, 0);
    pb->arrived = 0;
    pb->nthreads = nth;

    // clock_gettime(CLOCK_REALTIME, &pb->reference);
}

tspec pbarrier_wait(pbarrier_t *pb, tspec *offset) {
    pthread_mutex_lock(&pb->m);

    ++(pb->arrived);
    if (pb->arrived < pb->nthreads)
        pthread_cond_wait(&pb->c, &pb->m);
    else {
        clock_gettime(CLOCK_MONOTONIC, &pb->reference);
        pthread_cond_broadcast(&pb->c);
        pb->arrived = 0;
        // printf("Last arrived\n");
        // fflush(stdout);
    }

    pthread_mutex_unlock(&pb->m);

    if (offset == 0)
        return pb->reference;
    else {
        tspec wake_up;
        wake_up = tspec_add(&pb->reference, offset);
        // need to update activation! and start a new instance
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_up, 0);
        return wake_up;
    }
}

void gsem_wait(gsem_t *gs, int nsignals) {
    pthread_mutex_lock(&gs->m);
    gs->nsignals = nsignals;
    if (gs->narrived < gs->nsignals)
        pthread_cond_wait(&gs->c, &gs->m);
    gs->narrived = 0;
    gs->nsignals = 0;
    pthread_mutex_unlock(&gs->m);
}

void gsem_post(gsem_t *gs) {
    pthread_mutex_lock(&gs->m);
    gs->narrived++;
    if (gs->nsignals == gs->narrived)
        pthread_cond_signal(&gs->c);
    pthread_mutex_unlock(&gs->m);
}

void gsem_init(gsem_t *gs) {
    pmux_create_pc(&gs->m, 99);
    pthread_cond_init(&gs->c, 0);
    gs->nsignals = 0;
    gs->narrived = 0;
}
