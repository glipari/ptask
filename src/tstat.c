#define _GNU_SOURCE
#include "tstat.h"
#include "ptask.h"

struct wcet_measure {
    tspec last;
    tspec wcet;
    tspec first;
    int num_instances;
};

static struct wcet_measure measures[MAX_TASKS];

void tstat_init(int i)
{
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &measures[i].first);
  measures[i].num_instances = 0;
  measures[i].last = measures[i].first;
}

void tstat_record(int i)
{
    tspec now;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
    tspec delta = tspec_sub(&now, &measures[i].last);
    if (tspec_cmp(&delta, &measures[i].wcet) > 0) 
	measures[i].wcet = delta;
    measures[i].last = now;
    measures[i].num_instances++;
}

tspec tstat_getwcet(i)
{
    return measures[i].wcet;
}

tspec tstat_getavg(i) 
{
    tspec res;
    tspec diff = tstat_gettotal(i);
    res.tv_sec = diff.tv_sec / measures[i].num_instances;
    diff.tv_nsec += (diff.tv_sec % measures[i].num_instances) * 1000000000L;
    res.tv_nsec = diff.tv_nsec / measures[i].num_instances;
    return res;
}

int tstat_getnuminstances(i)
{
    return measures[i].num_instances;
}

tspec tstat_gettotal(i)
{
    return tspec_sub(&measures[i].last, &measures[i].first);
}

