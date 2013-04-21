#include <stdio.h>
#include <ptask.h>

void mytask()
{
  int k, i = get_taskindex();
  for (k=0; k<3; ++k) {
    printf("Task %d arrived with period %d\n", i, task_period(i));
    wait_for_period();
  }
}

int main()
{
  task_spec_t tparam = TASK_SPEC_DFL;

  ptask_init(SCHED_FIFO);

  printf("Before creating the task: act_flag = %d, wait_flag = %d\n",
	 tparam.act_flag, tparam.wait_flag);

  // setting the parameters
  tparam = (task_spec_t) {
    .period=tspec_from(1, SEC), 
    .rdline=tspec_from(1, SEC),
    .priority = 1, .processor = 1,
    .act_flag = ACT, .wait_flag = 1
  };

  printf("After setting the parameters: act_flag = %d, wait_flag = %d\n",
	 tparam.act_flag, tparam.wait_flag);
  
  int index = task_create_ex(&tparam, mytask);
  if (index >= 0) printf("Task %d created\n", index);

  pthread_join(get_threadid(index), 0);

  printf("Task completed!\n");

  return 0;
}
