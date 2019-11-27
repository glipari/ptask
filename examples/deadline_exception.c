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
            ptask_wait_for_period();
            printf("Task %d returned from a longjmp :\n\tCurrent time %ld | "
                   "Deadline %ld\n",
                   idx, ptask_gettime(MILLI),
                   tspec_to_rel(&ptask_get_task(idx)->dl, MILLI));
            continue;
        }
        printf("Task %d starting for the %i time\n", idx, job);
        dle_timer_start(); // arming the deadline exception timer timer
        if ((idx % 2) == 1 && job > 2)
            work_for(work_time * 3, unit);
        else
            work_for(work_time, unit);
        dle_timer_stop(); // disarming the deadline exception timer timer
        printf("Task %d completed for the %i time\n", idx, job);
        ptask_wait_for_period();
    }
}

ptask task1(void) { taskbody(500, MILLI); }

ptask task2(void) { taskbody(800, MILLI); }

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
    dle_manager_init();

    /* Creating two tasks with different priorities */
    printf("Creation of a task: %d\n",
           start_task(MILLI, 2000, 2000, 10, task1));
    printf("Creation of a task: %d\n", start_task(MILLI, 2000, 2000, 5, task2));

    /* Delaying tasks' activation by 2 seconds*/
    ptask_activate_at(1, 2, SEC);
    ptask_activate_at(2, 2, SEC);

    for (;;)
        ;
    return 0;
}
