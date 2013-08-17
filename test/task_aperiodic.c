#include "ptask.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int cp = 0;
int ca = 0;

int aperiodic_id = 0;

void myperiodic()
{
    int i;
    for (i=0; i<4; i++) {
	printf("Periodic task, %d\n", i);
	cp++;
	ptask_activate(aperiodic_id); 
	ptask_wait_for_instance();
    }
}

void myaperiodic() 
{
    int i;
    for (i=0; i<4; i++) {
	ptask_wait_for_instance();
	printf("APeriodic task, %d\n", i);
	ca = cp + 1;
    }
}


int main() 
{
    ptask_init(SCHED_FIFO, GLOBAL, PRIO_INHERITANCE);

    task_spec_t params = TASK_SPEC_DFL;
    params.type = APERIODIC;
    params.priority = 2;
    params.act_flag = NOW;
    
    printf("Creating aperiodic task");
    aperiodic_id = ptask_create_ex(myaperiodic, &params);

    if (aperiodic_id < 0) {
	printf("Cannot create aperiodic task\n");
	exit(-1);
    }
    
    params = TASK_SPEC_DFL;
    params.type = PERIODIC;
    params.period = tspec_from(1, SEC);
    params.priority = 1;
    params.act_flag = NOW;

    printf("Creating periodic task");
    int pid = ptask_create_ex(myperiodic, &params);
    if (pid < 0) {
	printf("Cannot create aperiodic task\n");
	exit(-1);
    }

    pthread_join(ptask_get_threadid(pid), 0);
    pthread_join(ptask_get_threadid(aperiodic_id), 0);

    assert(ca == 5);

    return 0;
}
