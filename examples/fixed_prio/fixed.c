#include <ptask.h>
#include <calibrate.h>
#include <stdlib.h>

void task()
{
    int dur = *((int*) ptask_get_argument());
    while (1) {
        //printf("Task with dur %d\n", dur);
        work_for(dur, MILLI);
        ptask_wait_for_period();
    }
}

struct task_pars {
    int wcet;
    int period;
    int deadline;
    int priority;
    int index;
};

#define STR_MAX 80

struct task_pars * load_tasks(char *filename)
{
    char line[STR_MAX+1];
    int i;
    int nlines = 0;

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Cannot open %s\n",filename);
        perror("");
        exit(-1);
    }
    
    while (fgets(line, STR_MAX, f) != NULL) nlines ++;
    printf("File contains %d tasks\n", --nlines);
    fseek(f, 0, SEEK_SET);

    struct task_pars *pars =
        (struct task_pars *)malloc((nlines+1)*sizeof(struct task_pars));

    for (i = 0; i<nlines; i++) {
        int ret = fscanf(f, "%d %d %d %d", &pars[i].wcet,
                         &pars[i].period,
                         &pars[i].deadline,
                         &pars[i].priority);
        if (ret != 4) {
            printf("Error in parsing file at line %d\n", i);
            perror("Error");
            exit(-1);
        }
        else {
            printf("Task %d : wcet %d, period %d, dline %d, prio %d\n",
                   i,
                   pars[i].wcet,
                   pars[i].period, 
                   pars[i].deadline,
                   pars[i].priority);
        }
    }
    pars[i].wcet = -1;
    
    return pars;
}



int main(int argc, char *argv[])
{
    
    if (argc < 2) {
        printf("Usage: %s input_file\n", argv[0]);
        exit(-1);
    }

    struct task_pars *pars = load_tasks(argv[1]);

    ptask_init(SCHED_FIFO, PARTITIONED, PRIO_INHERITANCE);
    calibrate();

    int i = 0;
    while (pars[i].wcet != -1) {
        tpars param = TASK_SPEC_DFL;
        param.period = tspec_from(pars[i].period, MILLI);
        param.rdline = tspec_from(pars[i].deadline, MILLI);
        param.priority = pars[i].priority;
        param.processor = 1;
        param.act_flag = DEFERRED;
        param.arg = &pars[i].wcet;

        pars[i].index = ptask_create_param(task, &param);
        i++;
    }
    int ntask = i;

    for (i=0; i<ntask; i++)
        ptask_activate_at(pars[i].index, 50, MILLI); 
    
    while(1);
    
}
