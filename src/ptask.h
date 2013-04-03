/*--------------------------------------------------------------*/
/*		PTASK LIBRARY - HEADER FILE			*/
/*--------------------------------------------------------------*/

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/*--------------------------------------------------------------*/

#define	MAX_TASKS	50
#define	MAX_GROUPS	10

/* time units for get_systime */
#define	SEC	1
#define	MILLI	2
#define	MICRO	3
#define	NANO	4

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
};

/*--------------------------------------------------------------*/
/*			TASK FUNCTIONS				*/
/*--------------------------------------------------------------*/

long	get_systime(int unit);
void	time_copy(struct timespec *td, struct timespec ts);
void	time_add_ms(struct timespec *t, unsigned int ms);
int	time_cmp(struct timespec t1, struct timespec t2);

void	ptask_init(int policy);
void	wait_for_activation(int i);
void	wait_for_period(int i);

int	task_argument(void* arg);
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

int	task_create(int i, void *(*task)(void *), int period, int drel, int prio, int aflag);
void	task_activate(int i);

/*--------------------------------------------------------------*/

