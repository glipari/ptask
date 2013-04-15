#include <pthread.h>
#include "ptask.h"

pthread_t	         _tid[MAX_TASKS];
struct task_par	         _tp[MAX_TASKS];
int                      first_free;
static pthread_mutex_t   _tp_mutex; /** this is used to protect the 
					_tp data structure from concurrent
					accesses from the main and the 
					threads */
#define _TP_BUSY    -2
#define _TP_NOMORE  -1

sem_t		   _tsem[MAX_TASKS];	/* for task_activate	*/

tspec_t ptask_t0;	        /* system start time		*/
int     ptask_policy;		/* common scheduling policy	*/

/**
   This function returns a free descriptor, or -1 if there are no more
   free descriptors. _tp is organised as a linked list, first_free is
   the head of the list. This extracts from the head. It uses the
   _tp_mutex to protect the critical section.
 */
static int allocate_tp()
{
    int x = first_free;
    if (x == _TP_NOMORE) return -1;
    else {
	pthread_mutex_lock(&_tp_mutex);
	if (_tp[x].free == _TP_BUSY) { 
	    pthread_mutex_unlock(&_tp_mutex);
	    return -1;
	}
	first_free = _tp[x].free;
	_tp[x].free = _TP_BUSY;
	pthread_mutex_unlock(&_tp_mutex);
	return x;
    }
}

/**
   Frees a descriptor. It inserts the free descriptor at the head of
   the queue. It uses the _tp_mutex to protect the critical section.
 */
static void release_tp(int i)
{
    pthread_mutex_lock(&_tp_mutex);
    
    _tp[i].free = first_free;
    first_free = i;
    
    pthread_mutex_unlock(&_tp_mutex);
}

// This is the task index as seen from the thread
static __thread int ptask_idx;

// this is to be called from the thread and returns the 
// current index
int get_taskindex() 
{
    return ptask_idx;
}

// the exit handler of each task
static void ptask_exit_handler(void *arg)
{
    release_tp(ptask_idx);
}

// the thread body. It does some bookkeeping and install the
// exit handler, then calls the real user task body 
// on exit, it cleans up everything
static void *ptask_std_body(void *arg)
{
    struct task_par *pdes = (struct task_par *)arg;
    pthread_cleanup_push(ptask_exit_handler, 0);

    ptask_idx = task_argument(arg);
    (*pdes->body)();
        
    pthread_cleanup_pop(1);

    return 0;
}

/*--------------------------------------------------------------*/
/*  PTASK_INIT: initialize some PTASK variables			*/
/*--------------------------------------------------------------*/

void ptask_init(int policy)
{
    int	i;

    ptask_policy = policy;

    /* initialize all private sem with the value 0	*/
    for (i=0; i<MAX_TASKS; i++) {
	sem_init(&_tsem[i], 0, 0);
	if (i == MAX_TASKS-1) 
	    _tp[i].free = _TP_NOMORE;
	else _tp[i].free = i+1;
    }
    first_free = 0;
    pthread_mutex_init(&_tp_mutex, 0);

    // initialize time
    tspec_init();
}

/*--------------------------------------------------------------*/
/*  WAIT_FOR_ACTIVATION: suspends the calling thread until the	*/
/*		     	 task_activation function is called	*/
/*		     	 and compute the next activation time	*/
/*--------------------------------------------------------------*/
void	wait_for_activation()
{

    /* suspend on a private semaphore */
    sem_wait(&_tsem[ptask_idx]);
}

void set_activation(const tspec_t *off)
{
    _tp[ptask_idx].at = tspec_add_delta(off, 
					_tp[ptask_idx].period, 
					MILLI);  
    _tp[ptask_idx].dl = tspec_add_delta(off, 
					_tp[ptask_idx].deadline, 
					MILLI);  
}


/*--------------------------------------------------------------*/
/*  WAIT_FOR_PERIOD: suspends the calling thread until the next	*/
/*		     activation and sets next activation time	*/
/*--------------------------------------------------------------*/
void	wait_for_period()
{

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, 
		    &(_tp[ptask_idx].at), NULL);

    /* when awaken, update next activation time */
    _tp[ptask_idx].at = tspec_add_delta(&(_tp[ptask_idx].at), 
					_tp[ptask_idx].period, MILLI);
    
    /* update absolute deadline */
    _tp[ptask_idx].dl = tspec_add_delta(&(_tp[ptask_idx].dl), 
					_tp[ptask_idx].period, MILLI);
}

/*--------------------------------------------------------------*/
/*  TASK_ARGUMENT: returns the argument of task i		*/
/*--------------------------------------------------------------*/

int task_argument(void* arg)
{
    struct task_par	*tp;
    
    tp = (struct task_par *)arg;
    return tp->arg;
}

/*--------------------------------------------------------------*/
/*  TASK_WCET: returns the WCET of task i			*/
/*--------------------------------------------------------------*/

long	task_wcet(int i)
{
    return _tp[i].wcet;
}

/*--------------------------------------------------------------*/
/*  TASK_PERIOD: returns the period of task i			*/
/*--------------------------------------------------------------*/

int	task_period(int i)
{
    return _tp[i].period;
}

/*--------------------------------------------------------------*/
/*  TASK_DEADLINE: returns the relative deadline of task i	*/
/*--------------------------------------------------------------*/

int	task_deadline(int i)
{
    return _tp[i].deadline;
}

/*--------------------------------------------------------------*/
/*  TASK_DMISS: returns the number of deadline misses		*/
/*--------------------------------------------------------------*/

int	task_dmiss(int i)
{
    return _tp[i].dmiss;
}

/*--------------------------------------------------------------*/
/*  TASK_ATIME: returns next activation time of task i in ms	*/
/*--------------------------------------------------------------*/

long	task_atime(int i)
{
    long	al;

    al = _tp[i].at.tv_sec*1000 + _tp[i].at.tv_nsec/1000000;
    return al;
}

/*--------------------------------------------------------------*/
/*  TASK_ABSDL: returns the absolute deadline of task i	in ms	*/
/*--------------------------------------------------------------*/

long	task_absdl(int i)
{
    long	dl;

    dl = _tp[i].dl.tv_sec*1000 + _tp[i].dl.tv_nsec/1000000;
    return dl;
}

/*--------------------------------------------------------------*/
/*  TASK_SETWCET: set the wcet of task i			*/
/*--------------------------------------------------------------*/

void	task_setwcet(int i, long wc)
{
    _tp[i].wcet = wc;
}

/*--------------------------------------------------------------*/
/*  TASK_SETPERIOD: set the period of task i			*/
/*--------------------------------------------------------------*/

void	task_setperiod(int i, int per)
{
	_tp[i].period = per;
}

/*--------------------------------------------------------------*/
/*  TASK_SETDEADLINE: set the relative deadline of task i	*/
/*--------------------------------------------------------------*/

void	task_setdeadline(int i, int dline)
{
    _tp[i].deadline = dline;
}

/*--------------------------------------------------------------*/
/*  DEADLINE_MISS: if a deadline is missed increments dmiss	*/
/*		   and returns 1, otherwise returns 0		*/
/*--------------------------------------------------------------*/

int	deadline_miss(int i)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    if (tspec_cmp(&now, &_tp[i].dl) > 0) {
	_tp[i].dmiss++;
	return 1;
    }
    return 0;
}

/*--------------------------------------------------------------*/
/*  TASK_CREATE: initialize thread parameters and creates a	*/
/*		 thread						*/
/*--------------------------------------------------------------*/
/**
   @todo add error handling through the use of errno.
 */
int task_create(
    void (*task)(void),
    int	period,
    int	drel,
    int	prio,
    int	aflag)
{
    pthread_attr_t	myatt;
    struct	sched_param mypar;
    int	tret;
    
    int i = allocate_tp();
    if (i == _TP_NOMORE) return -1;
    
    _tp[i].arg = i;
    _tp[i].wcet = 0;
    _tp[i].period = period;
    _tp[i].deadline = drel;
    _tp[i].priority = prio;
    _tp[i].dmiss = 0;
    _tp[i].body = task;

    pthread_attr_init(&myatt);
    if (ptask_policy != SCHED_OTHER)
	pthread_attr_setinheritsched(&myatt, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&myatt, ptask_policy);
    mypar.sched_priority = _tp[i].priority;
    pthread_attr_setschedparam(&myatt, &mypar);
    tret = pthread_create(&_tid[i], &myatt, 
			  ptask_std_body, (void*)(&_tp[i]));
    
    if (tret == 0) {
      if (aflag == ACT) task_activate(i);
      return i;
    }
    else {
      release_tp(i);
      return -1;
    } 
}

/*--------------------------------------------------------------*/
/*  TASK_ACTIVATE: activate task i				*/
/*--------------------------------------------------------------*/

void	task_activate(int i)
{
    struct timespec t;
    
    /* compute the absolute deadline */
    clock_gettime(CLOCK_MONOTONIC, &t);

    _tp[i].dl = t;
    _tp[i].dl = tspec_add_delta(&(_tp[i].dl), _tp[i].deadline, MILLI);

    /* compute the next activation time */
    _tp[i].at = t;
    _tp[i].at = tspec_add_delta(&(_tp[i].at), _tp[i].period, MILLI);

    /* send the activation signal */
    sem_post(&_tsem[i]);
}

/*--------------------------------------------------------------*/





