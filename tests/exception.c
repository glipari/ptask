#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>

#include "calibrate.h"
#include "ptask.h"

ptask taskbody(ptime work_time, int unit) {
    int idx = ptask_get_index(), job = 0;
    
    ptask_wait_for_activation();
    for (;; job++) {
        if (dle_chkpoint() != 0) {
            printf("*** CHKPOINT Task %d *** tid %d deadline %ld"
                   "| after longjmp: now %ld\n",
                   idx, ptask_get_task(idx)->tid,
                   tspec_to_rel(&ptask_get_task(idx)->dl, MILLI),
                   ptask_gettime(MILLI));

            ptask_wait_for_period();
            continue;
        }
        
        printf("Task %d starting the %i instance at time %ld with deadline %ld\n",
               idx, job, ptask_gettime(MILLI),
               tspec_to_rel(&ptask_get_task(idx)->dl, MILLI));
        
        dle_timer_start();
        /*if (idx > 1 && job > 2)
            work_for(work_time * 2, unit);
            else
            work_for(work_time, unit);*/
        work_for(work_time, unit);
        dle_timer_stop();

        printf("Task %d completed the %i instance at time %ld with deadline %ld\n",
               idx, job, ptask_gettime(MILLI),
               tspec_to_rel(&ptask_get_task(idx)->dl, MILLI));
        
        ptask_wait_for_period();
    }
}

ptask task1(void) { taskbody(500, MILLI); }

ptask task2(void) { taskbody(1000, MILLI); }

ptask task3(void) { taskbody(2500, MILLI); }

static int start_task(int unit, int period, int deadline, int priority,
                      void (*task_body)(void)) {
    tpars param;

    ptask_param_init(param);
    ptask_param_period(param, period, unit);
    ptask_param_deadline(param, deadline, unit);
    ptask_param_priority(param, priority);

    return ptask_create_param(task_body, &param);
}

int main(void) {
    ptask_init(SCHED_FIFO, PARTITIONED, PRIO_INHERITANCE);

    /* Initializes the deadline exception manager */
    dle_manager_init();

    /* Calibrate the work_for() function */
    calibrate();
    
    /* Creating three tasks with different priorities */
    printf("Creation of task: %d\n", start_task(MILLI, 2000, 2000, 10, task1));
    printf("Creation of task: %d\n", start_task(MILLI, 3000, 3000, 7, task2));
    printf("Creation of task: %d\n", start_task(MILLI, 5000, 5000, 5, task3));

    /* Delaying tasks' activation by 2 seconds*/
    ptask_activate_at(1, 2, SEC);
    ptask_activate_at(2, 2, SEC);
    ptask_activate_at(3, 2, SEC);

    for (;;)
        ;
    return 0;
}
