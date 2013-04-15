/*--------------------------------------------------------------*/
/*		PTASK LIBRARY - HEADER FILE			*/
/*--------------------------------------------------------------*/

#ifndef __PTASK_H__
#define __PTASK_H__

#include <pthread.h>
#include <semaphore.h>
#include <ptime.h>

/*--------------------------------------------------------------*/

#define	MAX_TASKS	50
#define	MAX_GROUPS	10

/* activation flag for task_create */
#define	NOACT		0
#define	ACT		1

/*--------------------------------------------------------------*/
struct task_par {
    int  arg;			/* task argument		*/
    long wcet;			/* task WCET in microseconds	*/
    int	 period;		/* task period in milliseconds	*/
    int	 deadline;		/* relative deadline in ms	*/
    int	 priority;		/* task priority in [0,99]	*/
    int  dmiss;			/* number of deadline miss	*/
    struct timespec at;		/* next activation time		*/
    struct timespec dl;		/* current absolute deadline	*/
    void (*body)(void);         /* the actual body of the task  */
    int free;                   /* >=0 if this descr is avail.  */
    int act_flag;               /* flag for postponed activ.    */
};

void	ptask_init(int policy); /** Initializes the library      */

/*---------------------------------------------------------------*/
/*			TASK FUNCTIONS                           */
/*---------------------------------------------------------------*/
void	wait_for_activation();  /** waits for an exp. activation */
void	wait_for_period();      /** waits for next periodic act. */
void    set_activation(const tspec_t *off); /** sets the act. time */
int     get_taskindex();        /** returns the thread own index */
int	task_argument(void* arg); /* TO BE REMOVED */

long	task_wcet(int i);
int	task_period(int i);
int	task_deadline(int i);
int	task_dmiss(int i);
long	task_atime(int i);
long	task_absdl(int i);

void	task_setwcet(int i, long wc);
void	task_setperiod(int i, int per);
void	task_setdeadline(int i, int dline);
int	deadline_miss(int i);

int	task_create(void (*task)(void), 
		    int period, int drel, int prio, int aflag);
void	task_activate(int i);

#endif

/*--------------------------------------------------------------*/

