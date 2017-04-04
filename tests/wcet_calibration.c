#include <ptask.h>
#include <pbarrier.h>
#include <tstat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

gsem_t sem;

#define WORK_NITER 10000000l
#define WORK() { int a = 1234; int b = 5679;      \
        for (int j=0; j<WORK_NITER; j++) a=(b*a); \
    }\
    
void calibrate()
{
    int i=0;
    
    for (i=0; i<50; i++) {

        WORK()
            
        ptask_wait_for_period();
    }
    gsem_post(&sem);

    return;
}


int main()
{
    ptask_init(SCHED_DEADLINE, GLOBAL, PRIO_INHERITANCE);
    gsem_init(&sem);

    tpars p;
    ptask_param_init(p);

    ptask_param_period(p, 100, MILLI);
    ptask_param_measure(p);
    ptask_param_deadline(p, 100, MILLI);
    p.runtime = tspec_from(30, MILLI);
    ptask_param_activation(p, NOW);
    
    // calibration
    int cal_index = ptask_create_param(calibrate, &p);
    
    gsem_wait(&sem, 1);
    tspec x = ptask_get_avg(cal_index);
    ptime x_micro = tspec_to(&x, MICRO);
    long n_iter = (WORK_NITER * 1000)/ x_micro;

    printf("Average exec time for %ld iterations = %ld microseconds\n", WORK_NITER, x_micro);
    printf("So, #iterations for 1 millisecond = %ld\n", n_iter);

    FILE *f = fopen("iterations.txt", "w");
    fprintf(f, "%ld", n_iter);
    fclose(f);
    
    assert(1);
    
    return 0;
}
