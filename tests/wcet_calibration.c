#include "calibrate.h"
#include <assert.h>
#include <pbarrier.h>
#include <ptask.h>
#include <stdio.h>
#include <stdlib.h>
#include <tstat.h>

gsem_t sem;

#define WORK_NITER 100000l

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

    fprintf(stdout, "Calibrating, wait ...\n");
    
    // calibration
    int cal_index = ptask_create_param(calibrate_task, &p);

    gsem_wait(&sem, 1);
    tspec x = ptask_get_avg(cal_index);
    ptime x_micro = tspec_to(&x, MICRO);
    int32_t n_iter = (((int32_t)WORK_NITER) * 1000) / x_micro;

    fprintf(stdout, "Average exec time for %ld iterations = %ld microseconds\n",
           WORK_NITER, x_micro);
    fprintf(stdout, "#iterations for 1 millisecond = %d\n", n_iter);

    FILE *f = fopen(CALIBRATE_FILE, "w");
    fprintf(f, "%d", n_iter);
    fclose(f);

    f = fopen(CALIBRATE_SHELL, "w");
    fprintf(f, "export %s=%d\n", PTASK_CALIBRATE_ITER, n_iter);
    fclose(f);

    fprintf(stdout, "Copy the content of %s into your .bashrc file\n"
            "or type source %s in your shell\n", PTASK_CALIBRATE_ITER, PTASK_CALIBRATE_ITER);

    assert(1);

    return 0;
}
