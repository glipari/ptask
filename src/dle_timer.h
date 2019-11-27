#ifndef __DLE_TIMER_H__
#define __DLE_TIMER_H__

#include "ptask.h"
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef MAX_TASKS
#define MAX_TASKS 50
#endif

struct dle_timer_s {
    int dle_timer_signo;   /*< defines the sigmask that the timer will
                               have to correspond to, in order to
                               throw an exception */
    pthread_t
    dle_timer_threadid;    /*< The timer is aimed to one specific task
                             (thread) */
    timer_t dle_timer_timerid; /*< This will be useful to arm / disarm
                                 the task’s timer */
    void (*dle_timer_handler)(int, siginfo_t *, void *); /*< handler executed upon timer expiration */
};

int dle_manager_init(); /*< Must be called in the main after ptask_init */

int dle_chkpoint();     /*< Set a check point */

int dle_timer_start(); /*< Start exception timer : it will expire at
                         task deadline */

int dle_timer_stop(); /*< Stop exception timer */

#endif
