#include "ptask.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int cp = 0;
int ca = 0;

int aperiodic_id = 0;

void myperiodic() {
    int i;
    for (i = 0; i < 4; i++) {
        printf("Periodic task, %d\n", i);
        cp++;
        ptask_activate(aperiodic_id);
        ptask_wait_for_period();
    }
}

void myaperiodic() {
    int i;
    for (i = 0; i < 4; i++) {
        ptask_wait_for_activation();
        printf("APeriodic task, %d\n", i);
        ca = cp + 1;
    }
}

int main() {
    ptask_init(SCHED_FIFO, GLOBAL, PRIO_INHERITANCE);

    tpars params = TASK_SPEC_DFL;
    params.priority = 2;
    params.act_flag = NOW;

    printf("Creating aperiodic task\n");
    aperiodic_id = ptask_create_param(myaperiodic, &params);

    if (aperiodic_id < 0) {
        printf("Cannot create aperiodic task\n");
        exit(-1);
    }

    params = TASK_SPEC_DFL;
    params.period = tspec_from(1, SEC);
    params.priority = 1;
    params.act_flag = NOW;

    printf("Creating periodic task\n");
    int pid = ptask_create_param(myperiodic, &params);
    if (pid < 0) {
        printf("Cannot create aperiodic task\n");
        exit(-1);
    }

    pthread_join(ptask_get_threadid(pid), 0);
    pthread_join(ptask_get_threadid(aperiodic_id), 0);

    assert(ca == 5);

    return 0;
}
