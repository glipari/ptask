#include "calibrate.h"
#include <assert.h>
#include <pbarrier.h>
#include <ptask.h>
#include <stdio.h>
#include <stdlib.h>
#include <tstat.h>

gsem_t sem;

#define WORK_NITER 10000000l

void calibrate_task() {
    int i = 0;

    for (i = 0; i < 50; i++) {

        WORK(WORK_NITER);

        ptask_wait_for_period();
    }
    gsem_post(&sem);

    return;
}

int main() {
    ptask_init(SCHED_DEADLINE, GLOBAL, PRIO_INHERITANCE);
    gsem_init(&sem);

    tpars p;
    ptask_param_init(p);

    ptask_param_period(p, 100, MILLI);
    ptask_param_measure(p);
    ptask_param_deadline(p, 100, MILLI);
    ptask_param_runtime(p, 30, MILLI);
    ptask_param_activation(p, NOW);

    // calibration
    int cal_index = ptask_create_param(calibrate_task, &p);

    gsem_wait(&sem, 1);
    tspec x = ptask_get_avg(cal_index);
    ptime x_micro = tspec_to(&x, MICRO);
    long n_iter = (WORK_NITER * 1000) / x_micro;

    printf("Average exec time for %ld iterations = %ld microseconds\n",
           WORK_NITER, x_micro);
    printf("So, #iterations for 1 millisecond = %ld\n", n_iter);

    FILE *f = fopen(CALIBRATE_FILE, "w");
    fprintf(f, "%ld", n_iter);
    fclose(f);

    assert(1);

    return 0;
}
