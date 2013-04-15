#include <stdio.h>
#include "pbarrier.h"

void pbarrier_init(pbarrier_t *pb, int nth)
{
    pthread_mutex_init(&pb->m, 0);
    pthread_cond_init(&pb->c, 0);
    pb->arrived = 0;
    pb->nthreads = nth;
  
    //clock_gettime(CLOCK_REALTIME, &pb->reference);
}

tspec_t pbarrier_wait(pbarrier_t *pb, tspec_t *offset)
{
    pthread_mutex_lock(&pb->m);

    ++(pb->arrived);
    if (pb->arrived < pb->nthreads) 
	pthread_cond_wait(&pb->c, &pb->m);
    else {
	clock_gettime(CLOCK_MONOTONIC, &pb->reference);
	pthread_cond_broadcast(&pb->c);
	pb->arrived = 0;
	printf("Last arrived\n");
	fflush(stdout);
    }

    pthread_mutex_unlock(&pb->m);

    if (offset == 0) return pb->reference;
    else {
	tspec_t wake_up;
	wake_up = tspec_add(&pb->reference, offset);
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_up, 0);
	return wake_up;
    }
}
