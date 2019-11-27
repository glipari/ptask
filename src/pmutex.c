#include "pmutex.h"

int pmux_create_pi(pthread_mutex_t *m) {
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_INHERIT);

    int ret = pthread_mutex_init(m, &mta);

    pthread_mutexattr_destroy(&mta);
    return ret;
}

int pmux_create_pc(pthread_mutex_t *m, int ceiling) {
    pthread_mutexattr_t mta;

    pthread_mutexattr_init(&mta);
    pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_PROTECT);
    pthread_mutexattr_setprioceiling(&mta, ceiling);

    int ret = pthread_mutex_init(m, &mta);

    pthread_mutexattr_destroy(&mta);
    return ret;
}

int pmux_destroy(pthread_mutex_t *m) { return pthread_mutex_destroy(m); }
