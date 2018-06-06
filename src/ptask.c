#define _GNU_SOURCE
#include "ptask.h"
#include "pmutex.h"
#include "tstat.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const tpars TASK_SPEC_DFL = {.runtime = {1, 0},
                             .period = {1, 0},
                             .rdline = {1, 0},
                             .priority = 1,
                             .processor = 0,
                             .act_flag = NOW,
                             .measure_flag = 0,
                             .arg = NULL,
                             .modes = NULL,
                             .nmodes = 0};

#define _TP_BUSY -2
#define _TP_NOMORE -1

pthread_t _tid[MAX_TASKS];
struct task_par _tp[MAX_TASKS];
static int first_free;
static pthread_mutex_t _tp_mutex; /** this is used to protect the
                                      _tp data structure from concurrent
                                      accesses from the main and the
                                      threads */
sem_t _tsem[MAX_TASKS];           /* for task_activate	      */
int ptask_policy;                 /* common scheduling policy   */
global_policy ptask_global;       /* global or partitioned      */
sem_protocol ptask_protocol;      /* semaphore protocol         */
static int ptask_num_cores;       /* number of cores in the system */

int dle_init(); // TODO : modify comment (before task initialization)

int dle_exit(); // TODO : modify comment (after task ends)

/**
   This function returns a free descriptor, or -1 if there are no more
   free descriptors. _tp is organised as a linked list, first_free is
   the head of the list. This extracts from the head. It uses the
   _tp_mutex to protect the critical section.
*/
static int allocate_tp() {
    int x = first_free;
    if (x == _TP_NOMORE)
        return -1;
    else {
        pthread_mutex_lock(&_tp_mutex);
        if (_tp[x].free == _TP_BUSY) {
            pthread_mutex_unlock(&_tp_mutex);
            return -1;
        }
        first_free = _tp[x].free;
        _tp[x].free = _TP_BUSY;

        if (ptask_protocol == PRIO_INHERITANCE)
            pmux_create_pi(&_tp[x].mux);
        else if (ptask_protocol == PRIO_CEILING)
            pmux_create_pc(&_tp[x].mux, 99);
        else
            pthread_mutex_init(&_tp[x].mux, 0);

        pthread_mutex_unlock(&_tp_mutex);
        return x;
    }
}

/**
   Frees a descriptor. It inserts the free descriptor at the head of
   the queue. It uses the _tp_mutex to protect the critical section.
 */
static void release_tp(int i) {
    pthread_mutex_lock(&_tp_mutex);

    _tp[i].free = first_free;
    pthread_mutex_destroy(&_tp[i].mux);
    first_free = i;

    pthread_mutex_unlock(&_tp_mutex);
}

// This is the task index as seen from the thread
static __thread int ptask_idx;

// this is to be called from the thread and returns the
// current index
int ptask_get_index() { return ptask_idx; }

// the exit handler of each task
static void ptask_exit_handler(void *arg) { release_tp(ptask_idx); }

// the thread body.
// 1) It does some book keeping and installs the
//    exit handler.
// 2) if necessary, waits for the first activation
// 3) then calls the real user task body
// 40 on exit, it cleans up everything
static void *ptask_std_body(void *arg) {
    struct task_par *pdes = (struct task_par *)arg;

    tspec t;

    ptask_idx = pdes->index;
    if (_tp[ptask_idx].measure_flag)
        tstat_init(ptask_idx);

    pthread_cleanup_push(ptask_exit_handler, 0);
    _tp[ptask_idx].tid = gettid();

    if (ptask_policy == SCHED_DEADLINE) {
        struct sched_attr attr;
        attr.size = sizeof(attr);
        attr.sched_policy = SCHED_DEADLINE;
        attr.sched_flags = SCHED_FLAG_RESET_ON_FORK;
        attr.sched_priority = 0;
        attr.sched_runtime = (__u64)tspec_to(&(_tp[ptask_idx].runtime), NANO);
        attr.sched_period = (__u64)tspec_to(&_tp[ptask_idx].period, NANO);
        attr.sched_deadline = (__u64)tspec_to(&_tp[ptask_idx].deadline, NANO);
        if (sched_setattr(_tp[ptask_idx].tid, &attr, 0) != 0) {
            printf("ERROR in setting sched_deadline parameters!\n");
            perror("Error:");
            return 0;
        }
        _tp[ptask_idx].schedattr = attr;
        // printf("SCHED_DEADLINE correctly set\n");
    }

    if (_tp[ptask_idx].act_flag == DEFERRED)
        ptask_wait_for_activation();
    else {
        clock_gettime(CLOCK_MONOTONIC, &t);
        _tp[ptask_idx].dl = tspec_add(&t, &_tp[ptask_idx].deadline);
        _tp[ptask_idx].at = tspec_add(&t, &_tp[ptask_idx].period);
    }

    dle_init();
    (*pdes->body)();
    dle_exit();
    pthread_cleanup_pop(1);

    return 0;
}

/*--------------------------------------------------------------*/
/*  PTASK_INIT: initialize some PTASK variables			*/
/*--------------------------------------------------------------*/

void ptask_init(int policy, global_policy global, sem_protocol protocol) {
    int i;

    ptask_policy = policy;
    ptask_global = global;
    ptask_protocol = protocol;
    ptask_num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    /* checks that admission control is disabled in case of partitioned edf */
    if ((ptask_policy == SCHED_DEADLINE) && (ptask_global == PARTITIONED)) {
        FILE *f = fopen("/proc/sys/kernel/sched_rt_runtime_us", "r");
        int v = 0;
        fscanf(f, "%d", &v);
        fclose(f);
        if (v != -1) {
            fprintf(stderr, "Cannot set PARTITIONED EDF scheduling, because "
                            "admission control is enabled\n");
            exit(-1);
        }
    }

    /* initialize all private sem with the value 0	*/
    for (i = 0; i < MAX_TASKS; i++) {
        sem_init(&_tsem[i], 0, 0);
        if (i == MAX_TASKS - 1)
            _tp[i].free = _TP_NOMORE;
        else
            _tp[i].free = i + 1;
    }
    first_free = 0;
    if (ptask_protocol == PRIO_INHERITANCE)
        pmux_create_pi(&_tp_mutex);
    else if (ptask_protocol == PRIO_CEILING)
        pmux_create_pc(&_tp_mutex, 99);
    else if (ptask_protocol == NO_PROTOCOL)
        pthread_mutex_init(&_tp_mutex, 0);
    else
        ptask_syserror("ptask_init()", "Semaphore protocol not supported");

    // initialize time
    tspec_init();
}

static int __create_internal(void (*task)(void), tpars *tp) {
    // pthread_attr_t	myatt;
    struct sched_param mypar;
    int tret;
    int j = 0;

    int i = allocate_tp();
    if (i == _TP_NOMORE)
        return -1;

    _tp[i].index = i;
    _tp[i].body = task;
    _tp[i].dmiss = 0;
    _tp[i].offset = tspec_zero;
    _tp[i].state = TASK_ACTIVE;
    _tp[i].cpu_id = -1;
    _tp[i].tid = -1;

    if (tp == NULL) {
        _tp[i].runtime = tspec_from(1, SEC);
        _tp[i].period = tspec_from(1, SEC);
        _tp[i].deadline = tspec_from(1, SEC);
        _tp[i].priority = (ptask_policy != SCHED_DEADLINE) ? 1 : 0;
        _tp[i].act_flag = DEFERRED;
        _tp[i].measure_flag = 0;
        _tp[i].arg = 0;
        _tp[i].modes = NULL;
    } else {
        _tp[i].runtime = tp->runtime;
        _tp[i].period = tp->period;
        _tp[i].deadline = tp->rdline;
        _tp[i].priority = tp->priority;
        _tp[i].act_flag = tp->act_flag;
        _tp[i].measure_flag = tp->measure_flag;
        _tp[i].arg = tp->arg;
        _tp[i].modes = tp->modes;
        if (tp->modes != NULL) {
            for (j = 0; j < tp->nmodes; ++j) {
                int result = rtmode_addtask(tp->modes, tp->mode_list[j], i);
                if (result == 0) {
                    release_tp(i);
                    return -1;
                }
            }
        }
    }

    pthread_attr_init(&_tp[i].attr);
    if (ptask_policy != SCHED_OTHER) {
        pthread_attr_setinheritsched(&_tp[i].attr, PTHREAD_EXPLICIT_SCHED);
    }

    if (ptask_policy != SCHED_DEADLINE) {
        pthread_attr_setschedpolicy(&_tp[i].attr, ptask_policy);
        mypar.sched_priority = _tp[i].priority;
        pthread_attr_setschedparam(&_tp[i].attr, &mypar);
    } else
        pthread_attr_setschedpolicy(&_tp[i].attr, SCHED_OTHER);
    cpu_set_t cpuset;
    if (ptask_global == PARTITIONED) {
        CPU_ZERO(&cpuset);
        CPU_SET(tp->processor, &cpuset);
        _tp[i].cpu_id = tp->processor;
        pthread_attr_setaffinity_np(&_tp[i].attr, sizeof(cpu_set_t), &cpuset);
    }

    tret = pthread_create(&_tid[i], &_tp[i].attr, ptask_std_body,
                          (void *)(&_tp[i]));
    pthread_attr_destroy(&_tp[i].attr);

    if (tret == 0) {
        return i;
    } else {
        release_tp(i);
        return -1;
    }
}

int ptask_create_param(void (*task)(void), tpars *tp) {
    return __create_internal(task, tp);
}

/*--------------------------------------------------------------*/
/*  TASK_CREATE: initialize thread parameters and creates a	*/
/*		 thread						*/
/*--------------------------------------------------------------*/
int ptask_create(void (*task)(void), int period, int prio, int aflag) {
    return ptask_create_prio(task, period, prio, aflag);
}

int ptask_create_prio(void (*task)(void), int period, int prio, int aflag) {
    tpars param = TASK_SPEC_DFL;
    param.period = tspec_from(period, MILLI);
    param.rdline = tspec_from(period, MILLI);
    param.priority = prio;
    param.act_flag = aflag;

    return __create_internal(task, &param);
}

int ptask_create_edf(void (*task)(void), int period, int runtime, int dline,
                     int aflag) {
    tpars param = TASK_SPEC_DFL;
    param.period = tspec_from(period, MILLI);
    param.runtime = tspec_from(runtime, MILLI);
    param.rdline = tspec_from(dline, MILLI);
    param.act_flag = aflag;

    return __create_internal(task, &param);
}

void ptask_wait_for_period() {
    pthread_mutex_lock(&_tp[ptask_idx].mux);
    if (_tp[ptask_idx].measure_flag)
        tstat_record(ptask_idx);

    if (_tp[ptask_idx].modes != NULL &&
        !rtmode_taskfind(_tp[ptask_idx].modes, ptask_idx)) {
        maxsem_post(&_tp[ptask_idx].modes->manager, &_tp[ptask_idx].at);
        pthread_mutex_unlock(&_tp[ptask_idx].mux);
        ptask_wait_for_activation();
        return;
    } else {
        _tp[ptask_idx].state = TASK_WFP;
        pthread_mutex_unlock(&_tp[ptask_idx].mux);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(_tp[ptask_idx].at),
                        NULL);
        pthread_mutex_lock(&_tp[ptask_idx].mux);
        _tp[ptask_idx].state = TASK_ACTIVE;
        /* update absolute deadline */
        _tp[ptask_idx].dl =
            tspec_add(&(_tp[ptask_idx].at), &_tp[ptask_idx].deadline);

        /* when awaken, update next activation time */
        _tp[ptask_idx].at =
            tspec_add(&(_tp[ptask_idx].at), &_tp[ptask_idx].period);

        pthread_mutex_unlock(&_tp[ptask_idx].mux);
        return;
    }
}

/*--------------------------------------------------------------*/
/*  WAIT_FOR_ACTIVATION: suspends the calling thread until the	*/
/*		     	 task_activation function is called	*/
/*		     	 and computes the next activation time	*/
/*--------------------------------------------------------------*/
void ptask_wait_for_activation() {
    /* suspend on a private semaphore */
    _tp[ptask_idx].state = TASK_SUSPENDED;
    // printf("before sem_wait on task %d\n", ptask_idx);
    sem_wait(&_tsem[ptask_idx]);
    // printf("after sem_wait on task %d\n", ptask_idx);
    pthread_mutex_lock(&_tp[ptask_idx].mux);
    _tp[ptask_idx].state = TASK_ACTIVE;
    if (_tp[ptask_idx].offset.tv_sec != 0 ||
        _tp[ptask_idx].offset.tv_nsec != 0) {
        pthread_mutex_unlock(&_tp[ptask_idx].mux);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                        &(_tp[ptask_idx].offset), NULL);
        pthread_mutex_lock(&_tp[ptask_idx].mux);
        _tp[ptask_idx].offset = tspec_zero;
    }
    pthread_mutex_unlock(&_tp[ptask_idx].mux);
}

/*--------------------------------------------------------------*/
/*  TASK_ARGUMENT: returns the argument of task i		*/
/*--------------------------------------------------------------*/
void *ptask_get_argument() { return _tp[ptask_idx].arg; }

ptask_state ptask_get_state(int i) { return _tp[i].state; }

pthread_attr_t *ptask_get_threadattr(int i) { return &_tp[i].attr; }

pthread_t ptask_get_threadid(int i) { return _tid[i]; }

struct task_par *ptask_get_task(int i) {
    return &_tp[i];
}

struct task_par *ptask_get_current() {
    return ptask_get_task(ptask_idx);
}

pthread_t running_thread_id() { return _tid[ptask_idx]; }

int ptask_get_period(int i, int unit) {
    int p;
    pthread_mutex_lock(&_tp[i].mux);
    p = tspec_to(&_tp[i].period, unit);
    pthread_mutex_unlock(&_tp[i].mux);
    return p;
}

void ptask_set_period(int i, int period, int unit) {
    tspec new_period;
    new_period = tspec_from(period, unit);

    pthread_mutex_lock(&_tp[i].mux);
    if (ptask_policy == SCHED_DEADLINE) {
        struct sched_attr *attr;
        attr = &_tp[i].schedattr;
        attr->sched_period = (__u64)tspec_to(&new_period, NANO);
        if (sched_setattr(_tp[i].tid, attr, 0) != 0) {
            attr->sched_period = (__u64)tspec_to(&_tp[i].period, NANO);
            printf("ERROR in ptask_set_period !\n");
            perror("Error:");
            return;
        }
    }
    _tp[i].period = new_period;
    pthread_mutex_unlock(&_tp[i].mux);
}

int ptask_get_deadline(int i, int unit) {
    int d;
    pthread_mutex_lock(&_tp[i].mux);
    d = tspec_to(&_tp[i].deadline, unit);
    pthread_mutex_unlock(&_tp[i].mux);
    return d;
}

void ptask_set_deadline(int i, int dline, int unit) {
    tspec new_dline;
    new_dline = tspec_from(dline, unit);

    pthread_mutex_lock(&_tp[i].mux);
    if (ptask_policy == SCHED_DEADLINE) {
        struct sched_attr *attr;
        attr = &_tp[i].schedattr;
        attr->sched_deadline = (__u64)tspec_to(&new_dline, NANO);
        if (sched_setattr(_tp[i].tid, attr, 0) != 0) {
            attr->sched_deadline = (__u64)tspec_to(&_tp[i].deadline, NANO);
            printf("ERROR in ptask_set_deadline !\n");
            perror("Error:");
            return;
        }
    }
    _tp[i].deadline = new_dline;
    pthread_mutex_unlock(&_tp[i].mux);
}

int ptask_get_runtime(int i, int unit) {
    int d;
    pthread_mutex_lock(&_tp[i].mux);
    d = tspec_to(&_tp[i].runtime, unit);
    pthread_mutex_unlock(&_tp[i].mux);
    return d;
}

void ptask_set_runtime(int i, int runtime, int unit) {
    tspec new_runtime;
    struct sched_attr *attr;

    if (ptask_policy != SCHED_DEADLINE) {
        printf("Error (ptask_set_runtime): The policy is not SCHED_DEADLINE\n");
        return;
    }

    new_runtime = tspec_from(runtime, unit);
    pthread_mutex_lock(&_tp[i].mux);
    attr = &_tp[i].schedattr;
    attr->sched_runtime = (__u64)tspec_to(&new_runtime, NANO);
    if (sched_setattr(_tp[i].tid, attr, 0) != 0) {
        attr->sched_runtime = (__u64)tspec_to(&_tp[i].runtime, NANO);
        printf("ERROR in ptask_set_runtime !\n");
        perror("Error:");
        return;
    }
    _tp[i].runtime = new_runtime;
    pthread_mutex_unlock(&_tp[i].mux);
}

int ptask_get_priority(int i) { return _tp[i].priority; }

void ptask_set_priority(int i, int prio) {
    struct sched_param mypar;

    if (ptask_policy == SCHED_DEADLINE) {
        printf("Error (ptask_set_priority): The policy is SCHED_DEADLINE\n");
        return;
    }
    _tp[i].priority = prio;
    mypar.sched_priority = prio;
    sched_setscheduler(ptask_get_threadid(i), ptask_policy, &mypar);
}

/* int	task_deadline(int i) */
/* { */
/*     return tspec_to(&_tp[i].deadline, MILLI); */
/* } */

/*--------------------------------------------------------------*/
/*  TASK_ATIME: returns next activation time of task i in ms	*/
/*--------------------------------------------------------------*/

/* long	task_atime(int i) */
/* { */
/*     long	al; */

/*     al = _tp[i].at.tv_sec*1000 + _tp[i].at.tv_nsec/1000000; */
/*     return al; */
/* } */

/*--------------------------------------------------------------*/
/*  TASK_ABSDL: returns the absolute deadline of task i	in ms	*/
/*--------------------------------------------------------------*/

/* long	task_absdl(int i) */
/* { */
/*     long	dl; */

/*     dl = _tp[i].dl.tv_sec*1000 + _tp[i].dl.tv_nsec/1000000; */
/*     return dl; */
/* } */

/*--------------------------------------------------------------*/
/*  TASK_SETPERIOD: set the period of task i			*/
/*--------------------------------------------------------------*/

/* void	task_setperiod(int i, int per) */
/* { */
/*     _tp[i].period = tspec_from(per, MILLI); */
/* } */

/*--------------------------------------------------------------*/
/*  TASK_SETDEADLINE: set the relative deadline of task i	*/
/*--------------------------------------------------------------*/

/* void	task_setdeadline(int i, int dline) */
/* { */
/*     _tp[i].deadline = tspec_from(dline, MILLI); */
/* } */

/*--------------------------------------------------------------*/
/*  DEADLINE_MISS: if a deadline is missed increments dmiss	*/
/*		   and returns 1, otherwise returns 0		*/
/*--------------------------------------------------------------*/

int ptask_deadline_miss() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (tspec_cmp(&now, &_tp[ptask_idx].dl) > 0)
        return 1;
    else
        return 0;
}

/*--------------------------------------------------------------*/
/*  TASK_ACTIVATE: activate task i				*/
/*--------------------------------------------------------------*/

int ptask_activate(int i) {
    struct timespec t;
    int ret = 1;
    pthread_mutex_lock(&_tp[i].mux);

    if (_tp[i].state == TASK_ACTIVE || _tp[i].state == TASK_WFP) {
        ret = -1;
    } else {
        clock_gettime(CLOCK_MONOTONIC, &t);

        /* compute the absolute deadline */
        _tp[i].dl = tspec_add(&t, &_tp[i].deadline);

        /* compute the next activation time */
        _tp[i].at = tspec_add(&t, &_tp[i].period);

        /* send the activation signal */
        sem_post(&_tsem[i]);
    }
    pthread_mutex_unlock(&_tp[i].mux);
    return ret;
}

int ptask_activate_at(int i, ptime offset, int unit) {
    tspec reloff = tspec_from(offset, unit);
    tspec t;
    int ret = 1;

    pthread_mutex_lock(&_tp[i].mux);

    /* if (_tp[i].state == TASK_ACTIVE || _tp[i].state == TASK_WFP) { */
    if (_tp[i].state == TASK_WFP) {
        ret = -1;
    } else {
        t = tspec_get_ref();
        /* compute the absolute deadline */
        _tp[i].offset = tspec_add(&t, &reloff);
        _tp[i].dl = tspec_add(&_tp[i].offset, &_tp[i].deadline);
        /* compute the next activation time */
        _tp[i].at = tspec_add(&_tp[i].offset, &_tp[i].period);
        /* send the activation signal */
        sem_post(&_tsem[i]);
        // printf("sem_post done on task %d\n", i);
    }
    pthread_mutex_unlock(&_tp[i].mux);
    return ret;
}

ptime ptask_get_nextactivation(int unit) {
    ptime at_tmp;
    tspec t = tspec_get_ref();

    pthread_mutex_lock(&_tp[ptask_idx].mux);
    at_tmp = tspec_to(&_tp[ptask_idx].at, unit) - tspec_to(&t, unit);
    pthread_mutex_unlock(&_tp[ptask_idx].mux);

    return at_tmp;
}

/*--------------------------------------------------------------*/

int ptask_migrate_to(int i, int core_id) {
    if (core_id >= ptask_num_cores)
        return -1;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = ptask_get_threadid(i); // pthread_self();
    _tp[i].cpu_id = core_id;
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int ptask_get_processor(int i) { return _tp[i].cpu_id; }

int ptask_getnumcores() { return ptask_num_cores; }

void ptask_syserror(char *f, char *msg) {
    fprintf(stderr, "%s: ", f);
    fprintf(stderr, "%s \n", msg);
    exit(-1);
}
