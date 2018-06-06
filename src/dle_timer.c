#include "dle_timer.h"
#include <stdlib.h>
#include <unistd.h>

#define MANAGER_TASK_INDEX 0

struct dle_manager_s {
    int dle_manager_tid;
    pthread_t dle_manager_threadid;
};

static struct dle_manager_s dle_manager;
static struct dle_timer_s dle_timers[MAX_TASKS];

static void dle_manager_handler(int signo, siginfo_t *info, void *context) {
    struct task_par *task = ptask_get_task(info->si_value.sival_int);
    printf("Handler Task Manager Warning : task %d received signal %d",
           info->si_value.sival_int, signo);
    printf("\n\tNow %ld, Deadline %ld\n", ptask_gettime(MILLI),
           tspec_to_rel(&task->dl, MILLI));
    if (syscall(SYS_tgkill, getpid(), task->tid, SIGUSR1) != 0)
        exit(EXIT_FAILURE);
}

static int dle_timer_initialized() {
    int task_index = ptask_get_index();
    return (dle_timers[task_index].dle_timer_timerid != 0);
}

static void task_handler(int sig, siginfo_t *info, void *context) {
    siglongjmp(ptask_get_current()->jmp_env, 1);
}

// Not meant to be calle the user
int dle_init() {
    struct sigaction manager_sigaction, task_sigaction;
    int task_index = ptask_get_index();
    struct sigevent sev;

    task_sigaction.sa_flags = SA_SIGINFO;
    task_sigaction.sa_sigaction = task_handler;
    sigemptyset(&task_sigaction.sa_mask);
    sigaddset(&task_sigaction.sa_mask, SIGUSR1);
    assert(sigaction(SIGUSR1, &task_sigaction, NULL) == 0);

    dle_timers[task_index].dle_timer_handler = dle_manager_handler;
    dle_timers[task_index].dle_timer_threadid =
        dle_manager.dle_manager_threadid;
    dle_timers[task_index].dle_timer_signo = SIGUSR2;

    manager_sigaction.sa_flags = SA_SIGINFO;
    manager_sigaction.sa_sigaction = dle_timers[task_index].dle_timer_handler;
    sigemptyset(&manager_sigaction.sa_mask);
    sigaddset(&manager_sigaction.sa_mask,
              dle_timers[task_index].dle_timer_signo);
    assert(sigaction(dle_timers[task_index].dle_timer_signo, &manager_sigaction,
                     NULL) == 0);

    sev.sigev_notify = SIGEV_THREAD_ID;
    sev.sigev_signo = dle_timers[task_index].dle_timer_signo;
    sev.sigev_value.sival_int = task_index;
    sev._sigev_un._tid = dle_manager.dle_manager_tid;

    return timer_create(CLOCK_MONOTONIC, &sev,
                        &dle_timers[task_index].dle_timer_timerid);
}

// Not meant to be called by the user
int dle_exit() {
    int task_index = ptask_get_index();
    if (!dle_timer_initialized())
        return -1;
    return timer_delete(dle_timers[task_index].dle_timer_timerid);
}

int dle_chkpoint() { return sigsetjmp(ptask_get_current()->jmp_env, 1); }

int dle_timer_start() {
    struct task_par *task = ptask_get_current();
    int task_index = task->index;
    struct itimerspec its;
    if (!dle_timer_initialized())
        return 1;

    its.it_value.tv_nsec = task->dl.tv_nsec;
    its.it_value.tv_sec = task->dl.tv_sec;
    its.it_interval.tv_nsec = its.it_interval.tv_sec = 0;

    printf("Arm %ld, task %d | Deadline %ld\n", ptask_gettime(MILLI),
           task_index, tspec_to_rel(&task->dl, MILLI));
    return timer_settime(dle_timers[task_index].dle_timer_timerid,
                         TIMER_ABSTIME, &its, NULL);
}

int dle_timer_stop() {
    struct itimerspec its;
    int task_index = ptask_get_index();
    if (!dle_timer_initialized())
        return 1;

    its.it_interval.tv_nsec = its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec = 0;

    printf("Disarm %ld, task %d | Deadline %ld\n", ptask_gettime(MILLI),
           task_index, tspec_to_rel(&ptask_get_task(task_index)->dl, MILLI));
    return timer_settime(dle_timers[task_index].dle_timer_timerid,
                         TIMER_ABSTIME, &its, NULL);
}

static ptask dle_manager_task(void) {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    dle_manager.dle_manager_tid = syscall(SYS_gettid);
    for (;;)
        pause();
}

int dle_manager_init() {
    tpars param;
    int res;
    ptask_param_init(param);
    ptask_param_priority(param, 99);
    res = ptask_create_param(dle_manager_task, &param);
    dle_manager.dle_manager_threadid = ptask_get_threadid(res);
    return res;
}
