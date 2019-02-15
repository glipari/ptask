//cmake ptask with -DTRACE=ON option

#include "ptask.h"
#include "calibrate.h"
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define PER 20  //task period
#define RNT 4   //task runtime
#define DREL 20 //task deadline
#define PRIO 30 //task priority
#define TT 1    //task forced time

void init() {
    srand(time(NULL));
    ptask_init(SCHED_DEADLINE, GLOBAL, PRIO_INHERITANCE);
}


void task() {
    while (1) {
        work_for(5, MILLI);
        ptask_wait_for_period();
    }
}

 
int main(void) {
    init();
    int i , j, k, l;
    
    tpars params = TASK_SPEC_DFL;
    params.period = tspec_from(PER, MILLI);
    params.rdline = tspec_from(DREL, MILLI);
    params.measure_flag = 1;
    params.processor = 0;
    params.act_flag = NOW;
    params.runtime = tspec_from(RNT, MILLI);
    params.priority = PRIO;
    
    i = ptask_create_param(task, &params);
    if (i != -1) printf("Task %d created and activated\n", i);
    else exit(-1);
    
    params.priority = PRIO - i;
    j = ptask_create_param(task, &params);
    if (j != -1) printf("Task %d created and activated\n", j);
    else exit(-1);
    
    params.priority = PRIO - j;
    k = ptask_create_param(task, &params);
    if (k != -1) printf("Task %d created and activated\n", k);
    else exit(-1);
    
    params.priority = PRIO - k;
    l = ptask_create_param(task, &params);
    if (l != -1) printf("Task %d created and activated\n", l);
    else exit(-1);

    while(1) ;
    
    return 0;
}
