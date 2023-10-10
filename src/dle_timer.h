#ifndef __DLE_TIMER_H__
#define __DLE_TIMER_H__

#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <ptask.h>

#ifndef MAX_TASKS
#define MAX_TASKS 50
#endif

/** Must be called in the main after ptask_init() */
int dle_manager_init();

/** Must be called at the start of each task that wants to check the
    deadlines */
int dle_init();

/** Must be called at the end of the task */
int dle_exit();

/** Macro for the chkpoint */
#define DLE_CHKPOINT sigsetjmp(ptask_get_current()->jmp_env, 1)

/** Starts exception timer : it will expire at task deadline */
int dle_timer_start();

/** Stops exception timer */
int dle_timer_stop(); 

#endif
