#include "rtmode.h"
#include "pmutex.h"
#include "ptask.h"
#include <stdio.h>
#include <stdlib.h>

tasklist_t tlist_sub(tasklist_t *a, tasklist_t *b) {
    int i, j;
    tasklist_t result;
    result.ntasks = 0;

    for (i = 0; i < a->ntasks; ++i) {
        for (j = 0; j < b->ntasks; ++j)
            if (a->task_list[i] == b->task_list[j])
                break;

        if (j == b->ntasks)
            result.task_list[result.ntasks++] = a->task_list[i];
    }
    return result;
}

void tasklist_init(tasklist_t *l) { l->ntasks = 0; }

int tasklist_add(tasklist_t *l, int taskid) {
    if (l->ntasks == RTMODE_MAX_TASKS)
        return 0;
    else {
        l->task_list[l->ntasks++] = taskid;
        return 1;
    }
}

static void mode_manager() {
    int i;
    rtmode_t *g = (rtmode_t *)ptask_get_argument();
    tspec wakeup;

    while (1) {
        ptask_wait_for_activation();
        int newmode = g->queue[g->tail++];
        // first step: count which tasks to block and
        // which tasks to activate
        tasklist_t toblock;
        tasklist_t towake;
        tasklist_init(&toblock);
        tasklist_init(&towake);

        if (g->curr_mode != -1 && newmode != -1) {
            toblock = tlist_sub(&g->modes[g->curr_mode], &g->modes[newmode]);
            towake = tlist_sub(&g->modes[newmode], &g->modes[g->curr_mode]);
        } else {
            if (g->curr_mode != -1)
                toblock = g->modes[g->curr_mode];
            if (newmode != -1)
                towake = g->modes[newmode];
        }
        // change mode
        g->curr_mode = newmode;
        if (toblock.ntasks != 0) {
            wakeup = maxsem_wait(&g->manager, toblock.ntasks);
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup, 0);
        }
        // all old tasks are now blocked, now activate all new tasks
        for (i = 0; i < towake.ntasks; ++i)
            ptask_activate(towake.task_list[i]);
        // all finished! we can sleep again
    }
}

int rtmode_init(rtmode_t *g, int nmodes) {
    int i;

    g->modes = (tasklist_t *)malloc(sizeof(tasklist_t) * nmodes);
    g->nmodes = nmodes;

    for (i = 0; i < nmodes; ++i)
        g->modes[i].ntasks = 0;

    g->curr_mode = -1;
    maxsem_init(&g->manager);
    g->head = g->tail = 0;

    tpars param = TASK_SPEC_DFL;
    param.priority = 99;
    param.arg = g;
    g->manager_id = ptask_create_param(mode_manager, &param);

    return g->manager_id;
}

int rtmode_addtask(rtmode_t *g, int modeid, int tid) {
    if (modeid >= g->nmodes)
        return 0;
    else
        return tasklist_add(&g->modes[modeid], tid);
}

void rtmode_changemode(rtmode_t *g, int new_mode_id) {
    g->queue[g->head++] = new_mode_id;
    ptask_activate(g->manager_id);
}

int rtmode_taskfind(rtmode_t *g, int tid) {
    int i;
    if (g->curr_mode < 0 || g->curr_mode >= RTMODE_MAX_MODES)
        return 0;

    for (i = 0; i < g->modes[g->curr_mode].ntasks; i++)
        if (tid == g->modes[g->curr_mode].task_list[i])
            return 1;

    return 0;
}

void maxsem_init(maxsem_t *gs) {
    pmux_create_pc(&gs->m, 99);
    pthread_cond_init(&gs->c, 0);
    gs->nsignals = 0;
    gs->narrived = 0;
}

void maxsem_post(maxsem_t *gs, tspec *t) {
    pthread_mutex_lock(&gs->m);
    if (tspec_cmp(t, &gs->max) > 0)
        gs->max = *t;
    gs->narrived++;
    if (gs->nsignals == gs->narrived)
        pthread_cond_signal(&gs->c);
    pthread_mutex_unlock(&gs->m);
}

tspec maxsem_wait(maxsem_t *gs, int nsignals) {
    pthread_mutex_lock(&gs->m);
    clock_gettime(CLOCK_MONOTONIC, &gs->max);
    gs->nsignals = nsignals;
    if (gs->narrived < gs->nsignals)
        pthread_cond_wait(&gs->c, &gs->m);
    gs->narrived = 0;
    gs->nsignals = 0;
    tspec ret = gs->max;
    pthread_mutex_unlock(&gs->m);
    return ret;
}
