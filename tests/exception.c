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
#include "dle_timer.h"

ptask taskbody(ptime work_time, int unit)
{
    dle_init();

    int idx = ptask_get_index(), job = 0;
    
    ptask_wait_for_activation();
    for (;; job++) {
        int r = DLE_CHKPOINT; 
        if (r != 0) {
            printf("    *** CHKPOINT Task %d *** tid %d deadline %ld"
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
            work_for(work_time * 3, unit);
        else
        work_for(work_time, unit);*/
        work_for(3*work_time, unit);
        dle_timer_stop();

        printf("Task %d completed the %i instance at time %ld with deadline %ld, r = %d\n",
               idx, job, ptask_gettime(MILLI),
               tspec_to_rel(&ptask_get_task(idx)->dl, MILLI), r);
        
        ptask_wait_for_period();
    }
    dle_exit();
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
    read_calibrate_env();

    int t1 = start_task(MILLI, 2000, 2000, 10, task1);
    int t2 = start_task(MILLI, 3000, 3000, 7, task2);
    int t3 = start_task(MILLI, 5000, 5000, 5, task3);
    
    /* Creating three tasks with different priorities */
    printf("Creation of task: %d tid : %d\n", t1, ptask_get_task(t1)->tid);
    printf("Creation of task: %d tid : %d\n", t2, ptask_get_task(t2)->tid);
    printf("Creation of task: %d tid : %d\n", t3, ptask_get_task(t3)->tid);

    /* Delaying tasks' activation by 2 seconds*/
    ptask_activate_at(t1, 2, SEC);
    ptask_activate_at(t2, 2, SEC);
    ptask_activate_at(t3, 2, SEC);

    for (;;)
        ;
    return 0;
}
