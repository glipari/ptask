#ifndef __RTMODE_H__
#define __RTMODE_H__

#include <pthread.h>
#include <ptime.h>

#define RTMODE_MAX_MODES 16
#define RTMODE_MAX_TASKS 16
#define RTMODE_MAX_REQUESTS 8

typedef struct rtmode {
    int task_list[RTMODE_MAX_TASKS]; /*< list of tasks               */
    int ntasks;                      /*< number of tasks in the list */
} tasklist_t;

void tasklist_init(tasklist_t *l);
int tasklist_add(tasklist_t *l, int taskid);

typedef struct semwmax {
    pthread_mutex_t m;
    pthread_cond_t c;
    int nsignals;
    int narrived;
    tspec max;
} maxsem_t;

typedef struct modegroup {
    tasklist_t *modes; /*< list of modes               */
    int nmodes;        /*< number of modes in the list */
    int curr_mode;     /*< index of current mode       */
    maxsem_t manager;  /*< semaphore for the manager   */
    int manager_id;    /*< id of task manager          */
    int queue[RTMODE_MAX_REQUESTS];
    int head, tail;
} rtmode_t;

/**
   Initializes the RT mode manager. This function should be called
   just after the ptask_init, to initialize all data structures for
   the mode manager.

   @param g: pointer to a struct rtmode_t
   @param nmodes: number of modes in the system
 */
int rtmode_init(rtmode_t *g, int nmodes);
int rtmode_addtask(rtmode_t *g, int modeid, int tid);

void rtmode_changemode(rtmode_t *g, int new_mode_id);
int rtmode_taskfind(rtmode_t *g, int id);

void maxsem_init(maxsem_t *s);
void maxsem_post(maxsem_t *gs, tspec *t);
tspec maxsem_wait(maxsem_t *gs, int nsignals);

#endif
