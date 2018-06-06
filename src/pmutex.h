#ifndef __PTASK_MUTEX_H__
#define __PTASK_MUTEX_H__

#define _GNU_SOURCE

#include <pthread.h>

int pmux_create_pi(pthread_mutex_t *m);
int pmux_create_pc(pthread_mutex_t *m, int ceiling);
int pmux_destroy(pthread_mutex_t *m);

#endif
