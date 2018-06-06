#ifndef __PBARRIER_H__
#define __PBARRIER_H__

#include <pthread.h>
#include <ptime.h>
#include <semaphore.h>

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
struct timespec pbarrier_wait(pbarrier_t *pb, tspec *offset);

/**
   Wait for n signals before unblocking the waiting task
 */
typedef struct gensem {
    pthread_mutex_t m;
    pthread_cond_t c;
    int nsignals;
    int narrived;
} gsem_t;

void gsem_wait(gsem_t *gs, int nsignals);
void gsem_post(gsem_t *gs);
void gsem_init(gsem_t *gs);

#endif
