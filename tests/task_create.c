#include <ptask.h>
#include <stdio.h>
#include <unistd.h>

void mytask() {
    int k, i = ptask_get_index();
    for (k = 0; k < 3; ++k) {
        printf("Task %d arrived with period %d\n", i,
               ptask_get_period(i, MILLI));
        ptask_wait_for_period();
    }
}

int main() {
    tpars tparam = TASK_SPEC_DFL;

    ptask_init(SCHED_FIFO, PARTITIONED, PRIO_INHERITANCE);

    printf("Before creating the task: act_flag = %d\n", tparam.act_flag);

    // setting the parameters
    tparam = (tpars){.period = tspec_from(1, SEC),
                     .rdline = tspec_from(1, SEC),
                     .priority = 1,
                     .processor = 1,
                     .act_flag = NOW};

    printf("After setting the parameters: act_flag = %d\n", tparam.act_flag);

    int index = ptask_create_param(mytask, &tparam);
    if (index >= 0)
        printf("Task %d created\n", index);

    pthread_join(ptask_get_threadid(index), 0);

    tparam = (tpars){.period = tspec_from(1, SEC),
                     .rdline = tspec_from(1, SEC),
                     .priority = 1,
                     .processor = 1,
                     .act_flag = 0};

    printf("After setting the parameters: act_flag = %d\n", tparam.act_flag);

    index = ptask_create_param(mytask, &tparam);
    if (index >= 0)
        printf("Task %d created\n", index);

    sleep(2);

    printf("Now activating\n");
    ptask_activate(index);

    pthread_join(ptask_get_threadid(index), 0);

    printf("Task completed!\n");

    return 0;
}
