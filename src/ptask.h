/*--------------------------------------------------------------*/
/*		PTASK LIBRARY - HEADER FILE			*/
/*--------------------------------------------------------------*/

#ifndef __PTASK_H__
#define __PTASK_H__

#include <pthread.h>
#include <semaphore.h>
#include <ptime.h>
#include <rtmode.h>

/*--------------------------------------------------------------*/

#define	MAX_TASKS	50
#define	MAX_GROUPS	10

/* activation flag for task_create */
#define	DEFERRED	0
#define	NOW		1

typedef enum {PARTITIONED, GLOBAL} global_policy;
typedef enum {PRIO_INHERITANCE, PRIO_CEILING, NO_PROTOCOL} sem_protocol;
typedef enum {PERIODIC, APERIODIC} ptask_type;

/**
   This structure is used to simplify the creation of a task by
   setting standard arguments
 */
typedef struct {
    ptask_type type; 
    tspec period; 
    tspec rdline;
    int priority;              /*< from 0 to 99                         */
    int processor;             /*< processor id                         */
    int act_flag;              /*< NOW if the create activates the task */
    
    int measure_flag;          /*< if 1, activates measure of exec time */
    void *arg;                 /*< pointer to a task argument           */
    rtmode_t *modes;           /*< a pointer to the mode handler        */
    int mode_list[RTMODE_MAX_MODES];  /*< the maximum number of modes   */
    int nmodes;               /*< num of modes in which the task is act */
} task_spec_t;

extern const task_spec_t TASK_SPEC_DFL;

/* ------------------------------------------------------------------ */
/*                     GLOBAL FUNCTIONS                               */
/* ------------------------------------------------------------------ */
int  ptask_getnumcores(); /* returns the number of available cores    */

/** The following function initializes the library. The policy can be:
    - SCHED_FIFO      fixed priority, fifo for same priority tasks
    - SCHED_RR        fixed priority, round robin for same priority tasks 
    - SCHED_OTHER     classical Linux scheduling policy (background) 
    
    The global policy can be:
    - GLOBAL          global scheduling with migration
    - PARTITIONED     partitioned scheduling (no migration)

    The semaphore protocol can be:
    - PRIO_INHERIT
    - PRIO_CEILING
*/ 
void  ptask_init(int policy,
		 global_policy global, 
		 sem_protocol protocol); 

/** Prints an error message on stderr and exits. The first string
    should contain the name of the failing function, the second string
    should contain a description of the error */
void  ptask_syserror(char *fun, char *msg); 


/*--------------------------------------------------------------------- */
/*			TASK CREATION                                   */
/*----------------------------------------------------------------------*/
int  ptask_create(void (*task)(void),
		  ptask_type type,
		  int period, int prio, int aflag);

int   ptask_create_ex(void (*task)(void), task_spec_t *tp);

/*-------------------------------------------------------------------------- */
/*			TASK FUNCTIONS                                       */
/*---------------------------------------------------------------------------*/
void      ptask_wait_for_instance(); /** waits for next period or activation */
void	  ptask_wait_for_activation(); /** waits for an exp. activation      */
int       ptask_migrate_to(int core_id); /** migrate task to processor       */
int       ptask_get_index();        /** returns the task own index           */
int	  ptask_deadline_miss();    /** true is the task missed its deadline */

void      set_activation(const tspec *off); /** sets the act. time           */

/* Global functions on tasks */
void	  ptask_activate(int i); /** activates the task of idx i              */

pthread_t ptask_get_threadid(int i);    /** returns the thread own id              */

void	  task_setdeadline(int i, int dline);
void *    task_argument();
void	  task_setperiod(int i, int per);
int	  task_period(int i);
int	  task_deadline(int i);
void	  task_setdeadline(int i, int dline);
long	  task_atime(int i);
long	  task_absdl(int i);


#endif

/*--------------------------------------------------------------*/

