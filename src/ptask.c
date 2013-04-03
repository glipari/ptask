/*--------------------------------------------------------------*/
/*  Library for PERIODIC TASKS using clock_nanosleep		*/
/*--------------------------------------------------------------*/

#include <pthread.h>
#include <time.h>
#include "ptask.h"

pthread_t	   _tid[MAX_TASKS];
struct task_par	   _tp[MAX_TASKS];

sem_t		   _tsem[MAX_TASKS];	/* for task_activate	*/
pthread_barrier_t  _barg[MAX_GROUPS];	/* for group_activate	*/

struct	timespec ptask_t0;	/* system start time		*/
int	ptask_policy;		/* common scheduling policy	*/


/*--------------------------------------------------------------*/
/*  GET_TIME_MS:	return the number of time units		*/
/*			since system start time			*/
/*--------------------------------------------------------------*/

long	get_systime(int unit)
{
struct timespec t;
long	tu, mul, div;

	switch (unit) {

	    case SEC:	mul = 1;
			div = 1000000000;
			break;

	    case MILLI:	mul = 1000;
			div = 1000000;
			break;

	    case MICRO:	mul = 1000000;
			div = 1000;
			break;

	    case NANO:	mul = 1000000000;
			div = 1;
			break;

	}

	clock_gettime(CLOCK_MONOTONIC, &t);
	tu = (t.tv_sec - ptask_t0.tv_sec) * mul;
	tu += (t.tv_nsec - ptask_t0.tv_nsec)/div;

	return tu;
}

/*--------------------------------------------------------------*/
/*  TIME_COPY:	copy time t1 in t2				*/
/*--------------------------------------------------------------*/

void	time_copy(struct timespec *td, struct timespec ts)
{
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

/*--------------------------------------------------------------*/
/*  TIME_ADD_MS:	add ms milliseconds to timespec t	*/
/*--------------------------------------------------------------*/

void	time_add_ms(struct timespec *t, unsigned int ms)
{
	t->tv_sec += ms/1000;
	t->tv_nsec += (ms%1000)*1000000;

	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

/*--------------------------------------------------------------*/
/*  TIME_CMP:	time compare					*/
/*--------------------------------------------------------------*/

int	time_cmp(struct timespec t1, struct timespec t2)
{
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

/*--------------------------------------------------------------*/
/*  PTASK_INIT: initialize some PTASK variables			*/
/*--------------------------------------------------------------*/

void	ptask_init(int policy)
{
int	i;

	ptask_policy = policy;

	/* initialize all private sem with the value 0	*/
	for (i=0; i<MAX_TASKS; i++)
		sem_init(&_tsem[i], 0, 0);

	clock_gettime(CLOCK_MONOTONIC, &ptask_t0);
}

/*--------------------------------------------------------------*/
/*  WAIT_FOR_ACTIVATION: suspends the calling thread until the	*/
/*		     	 task_activation function is called	*/
/*		     	 and compute the next activation time	*/
/*--------------------------------------------------------------*/

void	wait_for_activation(int i)
{

	/* suspend on a private semaphore */
	sem_wait(&_tsem[i]);
}

/*--------------------------------------------------------------*/
/*  WAIT_FOR_PERIOD: suspends the calling thread until the next	*/
/*		     activation and sets next activation time	*/
/*--------------------------------------------------------------*/

void	wait_for_period(int i)
{

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(_tp[i].at), NULL);

	/* when awaken, update next activation time */
	time_add_ms(&(_tp[i].at), _tp[i].period);

	/* update absolute deadline */
	time_add_ms(&(_tp[i].dl), _tp[i].period);
}

/*--------------------------------------------------------------*/
/*  TASK_ARGUMENT: returns the argument of task i		*/
/*--------------------------------------------------------------*/

int	task_argument(void* arg)
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

	if (time_cmp(now, _tp[i].dl) > 0) {
		_tp[i].dmiss++;
		return 1;
	}
	return 0;
}

/*--------------------------------------------------------------*/
/*  TASK_CREATE: initialize thread parameters and creates a	*/
/*		 thread						*/
/*--------------------------------------------------------------*/

int	task_create(
		int	i,
		void*	(*task)(void *),
		int	period,
		int	drel,
		int	prio,
		int	aflag)
{
pthread_attr_t	myatt;
struct	sched_param mypar;
int	tret;

	_tp[i].arg = i;
	_tp[i].wcet = 0;
	_tp[i].period = period;
	_tp[i].deadline = drel;
	_tp[i].priority = prio;
	_tp[i].dmiss = 0;

	pthread_attr_init(&myatt);
	if (ptask_policy != SCHED_OTHER)
		pthread_attr_setinheritsched(&myatt, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&myatt, ptask_policy);
	mypar.sched_priority = _tp[i].priority;
	pthread_attr_setschedparam(&myatt, &mypar);
	tret = pthread_create(&_tid[i], &myatt, task, (void*)(&_tp[i]));

	if (aflag == ACT) task_activate(i);
	return tret;
}

/*--------------------------------------------------------------*/
/*  TASK_ACTIVATE: activate task i				*/
/*--------------------------------------------------------------*/

void	task_activate(int i)
{
struct timespec t;

	/* compute the absolute deadline */
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(_tp[i].dl), t);
	time_add_ms(&(_tp[i].dl), _tp[i].deadline);

	/* compute the next activation time */
	time_copy(&(_tp[i].at), t);
	time_add_ms(&(_tp[i].at), _tp[i].period);

	/* send the activation signal */
	sem_post(&_tsem[i]);
}

/*--------------------------------------------------------------*/





