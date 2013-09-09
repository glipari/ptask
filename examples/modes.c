#include "ptask.h"
#include <stdio.h>

#define MODE_OFF   0
#define MODE_ON    1
#define MODE_FAIL  2

ptask taskbody()
{
    ptask_wait_for_activation();
    while (1) {
	printf("Task T%d is running\n", ptask_get_index());
	ptask_wait_for_period();
    }
}

int main()
{
    rtmode_t mymodes;
    tpars param;
    int res;

    ptask_init(SCHED_FIFO, GLOBAL, PRIO_INHERITANCE);
    
    res = rtmode_init(&mymodes, 3);
    if (res < 0) {
	printf("Cannot create the mode manager! \n"); 
    }

    param = TASK_SPEC_DFL;
    param.period = tspec_from(1, SEC);
    param.rdline = param.period;
    param.priority = 4;
    param.modes = &mymodes;
    param.nmodes = 2;
    param.mode_list[0] = MODE_ON;
    param.mode_list[1] = MODE_FAIL;
    res = ptask_create_param(taskbody, &param);

    printf("Creation of first task: %d\n", res);
    
    param = TASK_SPEC_DFL;
    param.period = tspec_from(2, SEC);
    param.rdline = param.period;
    param.priority = 3;
    param.modes = &mymodes;
    param.nmodes = 1;
    param.mode_list[0] = MODE_ON;
    res = ptask_create_param(taskbody, &param);
    
    printf("Creation of second task: %d\n", res);

    param = TASK_SPEC_DFL;
    param.period = tspec_from(500, MILLI);
    param.rdline = param.period;
    param.priority = 5;
    param.modes = &mymodes;
    param.nmodes = 1;
    param.mode_list[0] = MODE_FAIL;
    res = ptask_create_param(taskbody, &param);
    
    printf("Creation of third task: %d\n", res);

    rtmode_changemode(&mymodes, MODE_OFF);
    
    printf("Press: o (mode ON); p (mode OFF); x (mode FAIL); z (repair) q (quit)\n");
    char c = getchar();
    int curr = MODE_OFF;
    while (c != 'q') {
	printf("Pressed %c\n", c);
	switch(c) {
	case 'o' : if (curr == MODE_OFF) {
		printf("Turning on...\n");
		rtmode_changemode(&mymodes, MODE_ON);
		curr = MODE_ON;
	    }
	    break;
	case 'p' : if (curr == MODE_ON) {
		printf("Turning off...\n");
		rtmode_changemode(&mymodes, MODE_OFF);
		curr = MODE_OFF;
	    } 
	    break;
	case 'x': if (curr == MODE_ON) {
		printf("FAILURE!!\n");
		rtmode_changemode(&mymodes, MODE_FAIL);
		curr = MODE_FAIL;
	    }
	    break;
	case 'z': if (curr == MODE_FAIL) {
		printf("Repair\n");
		rtmode_changemode(&mymodes, MODE_ON);
		curr = MODE_ON;
	    }
	    break;
	default : printf("Key not used\n");
	    break;
	}
	printf("Press: o (mode ON); p (mode OFF); x (mode FAIL); z (repair) q (quit)\n");
	c = getchar();
    }
    return 0;
}
