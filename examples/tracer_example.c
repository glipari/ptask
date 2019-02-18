//cmake ptask with -DTRACE=ON option

#include "ptask.h"
#include "calibrate.h"
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define PER 50  //task period
#define RNT 20   //task runtime
#define DREL 25 //task deadline
#define PRIO 30 //task priority
#define TT 1    //task forced time

void init() {
    srand(time(NULL));
    ptask_init(SCHED_DEADLINE, GLOBAL, PRIO_INHERITANCE);
    calibrate();
}


void task() {
    for (int i=0; i<1000; i++) {
        work_for(5, MILLI);
        //printf("Done\n");
        ptask_wait_for_period();
    }
}


void aper_task() {
    while (1) {
        work_for(5, MILLI);
        ptask_wait_for_activation();
    }
}

void activ_aper_task()
{
    int task_to_activate = *((int*) ptask_get_argument());
    int i = 0;
    while(1) {
        if (i%3 == 0) work_for(15, MILLI);
        else work_for(5, MILLI);
        
        if (i%2 == 0) ptask_activate(task_to_activate);
        
        i++;
        ptask_wait_for_period();
    }
}

 
int main(void) {
    int i, j;
    int aper, act_aper;

    init();

    tpars params = TASK_SPEC_DFL;
    params.period = tspec_from(PER, MILLI);
    params.rdline = tspec_from(DREL, MILLI);
    params.measure_flag = 1;
    params.processor = 0;
    params.act_flag = DEFERRED;
    params.runtime = tspec_from(RNT, MILLI);
    params.priority = PRIO;
    
    i = ptask_create_param(task, &params);
    if (i != -1) printf("Task %d created and activated\n", i);
    else exit(-1);
    ptask_activate_at(i, 50, MILLI);
    
    params.priority = PRIO - i;
    j = ptask_create_param(task, &params);
    if (j != -1) printf("Task %d created and activated\n", j);
    else exit(-1);
    ptask_activate_at(j, 50, MILLI);
    
    /* params.priority = PRIO - j; */
    /* k = ptask_create_param(task, &params); */
    /* if (k != -1) printf("Task %d created and activated\n", k); */
    /* else exit(-1); */
    /* ptask_activate_at(k, 50, MILLI); */
    
    /* params.priority = PRIO - k; */
    /* l = ptask_create_param(task, &params); */
    /* if (l != -1) printf("Task %d created and activated\n", l); */
    /* else exit(-1); */
    /* ptask_activate_at(l, 50, MILLI); */

    /* params.priority = PRIO - l; */
    /* m = ptask_create_param(task, &params); */
    /* if (m != -1) printf("Task %d created and activated\n", m); */
    /* else exit(-1); */
    /* ptask_activate_at(m, 50, MILLI); */

    /* params.priority = PRIO - m; */
    /* n = ptask_create_param(task, &params); */
    /* if (n != -1) printf("Task %d created and activated\n", n); */
    /* else exit(-1); */
    /* ptask_activate_at(n, 50, MILLI); */

    aper = ptask_create_param(aper_task, &params);
    if (aper != -1) printf("Task %d created and activated\n", aper);
    else exit(-1);

    params.arg = &aper;
    act_aper = ptask_create_param(activ_aper_task, &params);
    if (act_aper != -1) printf("Task %d created and activated\n", act_aper);
    else exit(-1);
    ptask_activate_at(act_aper, 50, MILLI);
    
    while(1) ;
    
    return 0;
}
