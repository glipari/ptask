#include <stdio.h>
#include <ptask.h>
#include <pbarrier.h>
#include <ptime.h>

#define NTASKS 10

// Global variables
pbarrier_t barrier;
tspec offset[NTASKS];

//extern struct task_par _tp[MAX_TASKS];

void task_body()
{
    int i = get_taskindex();
    int k;
    tspec st;

    printf("[task %d] Init\n", i);
    printf("[task %d] Period: %d\n", i, task_period(i));
  
    for (k=0; k<1000000; ++k);

    printf("[TASK %d] waiting for offset\n", i);
  
    st = pbarrier_wait(&barrier, &offset[i]);
    
    printf("[TASK %d] Now can start! at time %ld\n", 
     	   i, tspec_to_rel(&st, MILLI)); 

    set_activation(&st);

    tspec temp = tspec_add_delta(&st, task_period(i), MILLI);
    printf("[TASK %d] start_time should be %ld\n", i,
	   tspec_to_rel(&temp, MILLI));
    

    while (1) {
	long now = tspec_gettime(MILLI); 
	printf("[TASK %d] Starting Cycle at time %ld\n", i, now);
	for (k=0; k<1000000; ++k);
	wait_for_instance();
    }
}


int main()
{
    int i;
    int ret;
    ptask_init(SCHED_FIFO, GLOBAL, PRIO_INHERITANCE);
    pbarrier_init(&barrier, NTASKS+1);
  
    for (i=0; i<NTASKS; i++) 
	offset[i] = tspec_from(i, SEC);

    for (i=0; i<NTASKS; i++) {
	ret = ptask_create(task_body, 
			   (i+1)*1000, 
			   (i+1)*1000, 
			  NTASKS + 1 - i, 
			  ACT);
	printf("ret = %d \n", ret);
    }
  
  
    // now all tasks have been creates, I am ready to start
    printf("Press a key and enter to start all tasks\n");

    char c = getchar();

    pbarrier_wait(&barrier, 0);

    printf("All tasks have started, now press a key + enter to finish");
    c = getchar();

    return 0;
}
