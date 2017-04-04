#include <ptask.h>
#include <pbarrier.h>
#include <stdio.h>
#include <assert.h>
#include "calibrate.h"


ptime periods[3]   = {100, 150, 180};
ptime deadlines[3] = {100, 150, 180};
ptime offsets[3]   = {10,  10,  10 };
int   wcet[3]      = {30,  60,  50 }; 
int   njobs[3]     = {5,   3,   3  }; 

pbarrier_t barrier;
gsem_t sem;

long iter_milli = 0; 

int cnts[3] = {0, 0, 0};

// the scheduling sequence
int trace_events[][3] = {
    {1, 0, 0},
    {1, 1, 0},
    {1, 1, 1},
    {2, 1, 1},
    {2, 2, 1},
    {3, 2, 1},
    {3, 2, 2},
    {4, 2, 2},
    {4, 3, 2},
    {5, 3, 2},
    {5, 3, 3}
};

int trace_exec(int idx)
{
    static int evt = 0;
    cnts[idx]++;
    printf("execution of task %d\n", idx);
    int flag = 1;
    for (int i=0; i<3; i++) {
        flag = flag && (cnts[i] == trace_events[evt][i]);
    }
    if (!flag) {
        printf("--> task period: %d\n", ptask_get_period(idx, MILLI));
        printf("--> task deadline: %d\n", ptask_get_deadline(idx, MILLI));
        printf("--> task processor: %d\n", ptask_get_processor(idx));
    }
    evt++;
    return flag;
}



void body()
{
    int idx = ptask_get_index();
    tspec off = tspec_from(offsets[idx], MILLI);
    
    pbarrier_wait(&barrier, &off);

    for (int i=0; i<njobs[idx]; i++) {

        WORK(wcet[idx] * iter_milli);
        // here we check that everything is ok
        assert(trace_exec(idx));
        ptask_wait_for_period();
    }
    gsem_post(&sem);
}

int main()
{
    tpars p;
    
    ptask_init(SCHED_DEADLINE, PARTITIONED, PRIO_INHERITANCE);
    gsem_init(&sem);
    pbarrier_init(&barrier, 3);
    iter_milli = calibrate();

    for (int i=0; i<3; i++) {
        ptask_param_init(p);
        ptask_param_period(p, periods[i], MILLI);
        ptask_param_deadline(p, deadlines[i], MILLI);
        ptask_param_processor(p, 0);
        p.runtime = tspec_from(wcet[i] + 1, MILLI);

        ptask_param_measure(p);
        ptask_param_activation(p, NOW);
        ptask_create_param(body, &p);
    }

    gsem_wait(&sem, 3);
    // the task have completed, success!
    
    return 0;
}
