#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "ptask.h"
#include "pmutex.h"
#include "tstat.h"

struct task_par {
    ptask_type type;    /* periodic or aperiodic        */
    void * arg;         /* task argument                */
    int   index;	/* task index		        */
    tspec period;	/* task period 	                */
    tspec deadline;	/* relative deadline 	        */
    int	  priority;	/* task priority in [0,99]	*/
    int   dmiss;	/* number of deadline misses	*/
    tspec at;		/* next activation time		*/
    tspec dl;		/* current absolute deadline	*/
    void (*body)(void); /* the actual body of the task  */
    int  free;          /* >=0 if this descr is avail.  */
    int  act_flag;      /* flag for postponed activ.    */
    int  measure_flag;  /* flag for measurement         */
    rtmode_t *modes;    /* the mode descripton          */
};


const task_spec_t TASK_SPEC_DFL = {
    .type = PERIODIC,
    .period = {1, 0},  
    .rdline = {1, 0},
    .priority = 1, 
    .processor = 0, 
    .act_flag = ACT, 
    .measure_flag = 0,
    .arg = NULL,
    .modes = NULL,
    .nmodes = 0
};


       pthread_t	 _tid[MAX_TASKS];
       struct task_par	 _tp[MAX_TASKS];
static int               first_free;
static pthread_mutex_t   _tp_mutex; /** this is used to protect the 
					_tp data structure from concurrent
					accesses from the main and the 
					threads */
#define _TP_BUSY    -2
#define _TP_NOMORE  -1

sem_t         _tsem[MAX_TASKS];	        /* for task_activate	      */
tspec         ptask_t0;	                /* system start time	      */
int           ptask_policy;		/* common scheduling policy   */
global_policy ptask_global;             /* global or partitioned      */
sem_protocol  ptask_protocol;           /* semaphore protocol         */
static int    ptask_num_cores;          /* number of cores in the system */


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

// the thread body.
// 1) It does some book keeping and installs the
//    exit handler.
// 2) if necessary, waits for the first activation
// 3) then calls the real user task body
// 40 on exit, it cleans up everything
static void *ptask_std_body(void *arg)
{
    struct task_par *pdes = (struct task_par *)arg;
    
    ptask_idx = pdes->index;
    if (_tp[ptask_idx].measure_flag)
	tstat_init(ptask_idx);

    pthread_cleanup_push(ptask_exit_handler, 0);

    if (_tp[ptask_idx].act_flag == NOACT)
      wait_for_activation();
    else
      clock_gettime(CLOCK_MONOTONIC, &_tp[ptask_idx].at);
    
    (*pdes->body)();
        
    pthread_cleanup_pop(1);

    return 0;
}

/*--------------------------------------------------------------*/
/*  PTASK_INIT: initialize some PTASK variables			*/
/*--------------------------------------------------------------*/

void ptask_init(int policy,
		global_policy global,
		sem_protocol protocol)
{
    int	i;

    ptask_policy = policy;
    ptask_global = global;
    ptask_protocol = protocol;
    ptask_num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    /* initialize all private sem with the value 0	*/
    for (i=0; i<MAX_TASKS; i++) {
	sem_init(&_tsem[i], 0, 0);
	if (i == MAX_TASKS-1)
	    _tp[i].free = _TP_NOMORE;
	else _tp[i].free = i+1;
    }
    first_free = 0;
    if (ptask_protocol == PRIO_INHERITANCE) pmux_create_pi(&_tp_mutex);
    else if (ptask_protocol == PRIO_CEILING) pmux_create_pc(&_tp_mutex, 99);
    else if (ptask_protocol == NO_PROTOCOL) pthread_mutex_init(&_tp_mutex, 0);
    else ptask_syserror("ptask_init()", "Semaphore protocol not supported");

    // initialize time
    tspec_init();
}


static int __create_internal(void (*task)(void), task_spec_t *tp)
{
    pthread_attr_t	myatt;
    struct	sched_param mypar;
    int	tret;
    int j=0;
    
    int i = allocate_tp();
    if (i == _TP_NOMORE) return -1;
    
    _tp[i].type = tp->type;
    _tp[i].index = i;
    _tp[i].period = tp->period;
    _tp[i].deadline = tp->rdline;
    _tp[i].priority = tp->priority;
    _tp[i].dmiss = 0;
    _tp[i].body = task;
    _tp[i].act_flag = tp->act_flag;
    _tp[i].measure_flag = tp->measure_flag;
    _tp[i].arg = tp->arg;
    _tp[i].modes = tp->modes;
    if (tp->modes != NULL) {
	for (j=0; j<tp->nmodes; ++j) { 
	    int result = rtmode_addtask(tp->modes, tp->mode_list[j], i);
	    if (result == 0) {
		release_tp(i);
		return -1;
	    }
	}
    }
    
    pthread_attr_init(&myatt);
    if (ptask_policy != SCHED_OTHER)
	pthread_attr_setinheritsched(&myatt, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&myatt, ptask_policy);
    mypar.sched_priority = _tp[i].priority;
    pthread_attr_setschedparam(&myatt, &mypar);

    cpu_set_t cpuset;
    if (ptask_global == PARTITIONED) {
      CPU_ZERO(&cpuset);
      CPU_SET(tp->processor, &cpuset);
      
      pthread_attr_setaffinity_np(&myatt, sizeof(cpu_set_t), &cpuset);
    }

    tret = pthread_create(&_tid[i], &myatt, 
			  ptask_std_body, (void*)(&_tp[i]));

    pthread_attr_destroy(&myatt);
    
    if (tret == 0) {
      //if (tp->act_flag == ACT) task_activate(i);
      return i;
    }
    else {
      release_tp(i);
      return -1;
    }  
}

int ptask_create_ex(void (*task)(void), task_spec_t *tp)
{
     return __create_internal(task, tp);
}


/*--------------------------------------------------------------*/
/*  TASK_CREATE: initialize thread parameters and creates a	*/
/*		 thread						*/
/*--------------------------------------------------------------*/
int ptask_create(
    void (*task)(void),
    int	period,
    int	drel,
    int	prio,
    int	aflag)
{
    task_spec_t param = TASK_SPEC_DFL;
    param.period = tspec_from(period, MILLI);
    param.rdline = tspec_from(drel, MILLI);
    param.priority = prio;
    param.act_flag = aflag;

    return __create_internal(task, &param);
}


static void __wait_for_period()
{
    if (_tp[ptask_idx].measure_flag)
	tstat_record(ptask_idx);
    
    if (_tp[ptask_idx].modes != NULL &&
	!rtmode_taskfind(_tp[ptask_idx].modes, ptask_idx)) {
	maxsem_post(&_tp[ptask_idx].modes->manager, &_tp[ptask_idx].at);
	wait_for_activation();
	return;
    }
    else {
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
			&(_tp[ptask_idx].at), NULL);
	
	/* when awaken, update next activation time */
	_tp[ptask_idx].at = tspec_add(&(_tp[ptask_idx].at),
				      &_tp[ptask_idx].period);
	
	/* update absolute deadline */
	_tp[ptask_idx].dl = tspec_add(&(_tp[ptask_idx].dl),
				      &_tp[ptask_idx].period);
	return;
    }
}

/*--------------------------------------------------------------*/
/*  WAIT_FOR_ACTIVATION: suspends the calling thread until the	*/
/*		     	 task_activation function is called	*/
/*		     	 and computes the next activation time	*/
/*--------------------------------------------------------------*/
void  wait_for_activation()
{
    /* suspend on a private semaphore */
    sem_wait(&_tsem[ptask_idx]);
}

void wait_for_instance()
{
    if (_tp[ptask_idx].type == PERIODIC) __wait_for_period();
    else if (_tp[ptask_idx].type == APERIODIC) wait_for_activation();
    else ptask_syserror("wait_for_instance()", "wrong type");
}
 
/*--------------------------------------------------------------*/
/*  TASK_ARGUMENT: returns the argument of task i		*/
/*--------------------------------------------------------------*/
void * task_argument()
{
    return _tp[ptask_idx].arg;
}

void set_activation(const tspec *t)
{
    _tp[ptask_idx].at = tspec_add(t, &_tp[ptask_idx].period);
    _tp[ptask_idx].dl = tspec_add(t, &_tp[ptask_idx].deadline);
}

pthread_t get_threadid(int i)
{
    return _tid[i];
}

/*--------------------------------------------------------------*/
/*  TASK_PERIOD: returns the period of task i			*/
/*--------------------------------------------------------------*/

int	task_period(int i)
{
    return tspec_to(&_tp[i].period, MILLI);
}

/*--------------------------------------------------------------*/
/*  TASK_DEADLINE: returns the relative deadline of task i	*/
/*--------------------------------------------------------------*/

int	task_deadline(int i)
{
    return tspec_to(&_tp[i].deadline, MILLI);
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
/*  TASK_SETPERIOD: set the period of task i			*/
/*--------------------------------------------------------------*/

void	task_setperiod(int i, int per)
{
    _tp[i].period = tspec_from(per, MILLI);
}

/*--------------------------------------------------------------*/
/*  TASK_SETDEADLINE: set the relative deadline of task i	*/
/*--------------------------------------------------------------*/

void	task_setdeadline(int i, int dline)
{
    _tp[i].deadline = tspec_from(dline, MILLI);
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
/*  TASK_ACTIVATE: activate task i				*/
/*--------------------------------------------------------------*/

void	task_activate(int i)
{
    struct timespec t;
    
    /* compute the absolute deadline */
    clock_gettime(CLOCK_MONOTONIC, &t);

    _tp[i].dl = tspec_add(&t, &_tp[i].deadline);

    /* compute the next activation time */
    _tp[i].at = tspec_add(&t, &_tp[i].period);

    /* send the activation signal */
    sem_post(&_tsem[i]);
}

/*--------------------------------------------------------------*/

int migrate_to(int core_id) 
{
    //int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id >= ptask_num_cores)
	return -1;
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    pthread_t current_thread = pthread_self();    
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int ptask_getnumcores()
{
    return ptask_num_cores;
}


void ptask_syserror(char *f, char *msg)
{
  fprintf(stderr, "%s: ", f);
  fprintf(stderr, "%s \n", msg);
  exit(-1);
}
