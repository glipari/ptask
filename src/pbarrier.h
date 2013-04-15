#ifndef __PBARRIER_H__
#define __PBARRIER_H__

#include <semaphore.h>
#include <pthread.h>
#include <ptime.h>

/**
   Structure of the barrier
 */
typedef struct pbarrier {
    pthread_mutex_t m;
    pthread_cond_t c;
    unsigned arrived;
    unsigned nthreads;
    struct timespec reference; 
} pbarrier_t;

/**
   Initializes the timed barrier with the expected number 
   of threads. 
 */
void pbarrier_init(pbarrier_t *pb, int nth);

/**
   The calling threads waits until the last of the nthreads arrives.
   From that instant, the thread waits for offset time before it can
   wake up and execute (calling a nanosleep).
 */
struct timespec pbarrier_wait(pbarrier_t *pb, tspec_t *offset);

#endif
