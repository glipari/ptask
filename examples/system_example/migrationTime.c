#define _GNU_SOURCE
#include "pmutex.h"
#include "ptask.h"
#include "tstat.h"
#include <stdio.h>
#include <stdlib.h>

int idx;

ptask taskbody_manager() {
    int cpu_old = 0, idCPU = 1;
    while (1) {
        printf("	# Task T%d is running\n", ptask_get_index());

        // migration
        cpu_set_t cpuset;
        ptime time_migration;
        pthread_t thread = ptask_get_threadid(idx);
        struct timespec now, finish, time_temp;
        CPU_ZERO(&cpuset);
        CPU_SET(idCPU, &cpuset);

        // time start migration
        clock_gettime(CLOCK_MONOTONIC, &now);

        // migration
        ptask_migrate_to(0, idCPU);

        // control if migration is terminated
        while (1) {
            if (pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset) == 0)
                break;
        }
        // time migration complete
        clock_gettime(CLOCK_MONOTONIC, &finish);

        time_temp = tspec_sub(&finish, &now);
        time_migration = tspec_to(&time_temp, MICRO);

        printf("	# Migration T%d from core=%d to core=%d performed in "
               "%ld (microSec)\n",
               idx, cpu_old, idCPU, time_migration);

        cpu_old = idCPU;
        idCPU = (idCPU + 1) % ptask_getnumcores();

        ptask_wait_for_period();
    }
}

ptask taskbody() {
    while (1) {
        printf("Task T%d is running on core %d\n", ptask_get_index(),
               sched_getcpu());
        ptask_wait_for_period();
    }
}

int main(void) {
    int sched = SCHED_OTHER;
    int prot = NO_PROTOCOL;
    int part = PARTITIONED;
    int period_Manager = 5000;        // ms
    int period_task_toMigrate = 1000; // ms
    int id;
    char c;
    tpars params;

    ptask_init(sched, part, prot);

    printf("-- MIGRATION TIME TEST --\n");

    // creation task to migrate
    params = TASK_SPEC_DFL;
    params.period = tspec_from(period_task_toMigrate, MILLI);
    params.priority = 70;
    params.measure_flag = 0;
    params.act_flag = NOW;
    params.processor = 0;
    idx = ptask_create_param(taskbody, &params);
    if (idx != -1)
        printf("Task %d created and activated\n", idx);
    else
        printf(" ERROR! task not created!!!\n");

    // creation task that perform migration
    params = TASK_SPEC_DFL;
    params.period = tspec_from(period_Manager, MILLI);
    params.priority = 80;
    params.measure_flag = 0;
    params.act_flag = NOW;
    params.processor = 0;
    id = ptask_create_param(taskbody_manager, &params);
    if (id != -1)
        printf("Task %d created and activated\n", id);
    else
        printf(" ERROR! task not created!!!\n");

    c = getchar();
    while (c != 'q') {
        printf("Pressed %c\n", c);
        printf("To exit press q\n");
        c = getchar();
    }

    return 0;
}
