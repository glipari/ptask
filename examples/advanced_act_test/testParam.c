#include <stdbool.h>
#include <string.h>

#include "animation.h"
#include "graphics.h"
#include "testParameters.h"

#define ATTIVO true
#define NON_ATTIVO false

long period[NUM_T_TEST];      /* task periods		*/
int rel_dl[NUM_T_TEST];       /* task deadline	*/
int prio[NUM_T_TEST];         /* task priority	*/
int act_flag[NUM_T_TEST];     /* activation flag = {0:NOW, 1:DEFERRED}*/
int processore[NUM_T_TEST];   /* id processor		*/
int measure_flag[NUM_T_TEST]; /* if 1, activates measure of exec time	*/
int colors[NUM_T_TEST];       /* ball's color */

bool
    state_task[NUM_T_TEST]; /*  vector that means if a taski is active or not */

int num_tasks;      /* number of tasks	*/
int scale = MILLI;  /* time scale		*/
int dipPeriodo = 0; /* it means the 'dependency' of movement of ball from the
                       period(0:no, 1:yes) */

/* Inizialized Allegro and semaphores */
void init() {
    int i = 0;
    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    install_mouse();
    srand(time(NULL));

    for (i = 0; i < NUM_T_TEST; i++)
        state_task[i] = NON_ATTIVO;

    /* semaphore for access to screen in mutual exclusion */
    pmux_create_pi(&mxa);
}

/* Prints parameters of structure tpars */
void printf_param(tpars tp, const char *str, int unit) {
    printf("Struttura tpars %s\n", str);

    printf("Periodo  :\t%ld\n", tspec_to(&tp.period, unit));
    printf("Rel. dl  :\t%ld\n", tspec_to(&tp.rdline, unit));
    printf("Priority :\t%d\n", tp.priority);
    printf("act_flag :\t%d\n", tp.act_flag);
    printf("Processor:\t%d\n", tp.processor);
    printf("MeasureFl:\t%d\n", tp.measure_flag);
}

/* reads the task's parameters from file */
void get_data() {
    char c;
    int i;
    FILE *fp;

    fp = fopen("param.dat", "r");
    if (fp == NULL) {
        printf("File Param.DAT not found\n");
        exit(1);
    }

    do {
        c = getc(fp);
    } while (c != ':');
    fscanf(fp, "%d", &scale);

    do {
        c = getc(fp);
    } while (c != ':');
    fscanf(fp, "%d", &dipPeriodo);
    // i = 1;
    i = 0;
    do {
        do {
            c = getc(fp);
        } while ((c != ':') && (c != EOF));
        if (c != EOF) {
            fscanf(fp, "%ld", &period[i]);
            fscanf(fp, "%d", &rel_dl[i]);
            fscanf(fp, "%d", &prio[i]);
            fscanf(fp, "%d", &act_flag[i]);
            fscanf(fp, "%d", &processore[i]);
            fscanf(fp, "%d", &measure_flag[i]);
            fscanf(fp, "%d", &colors[i]);
            i++;
        }
    } while (c != EOF);

    num_tasks = i;
    fclose(fp);
}

/* given id_param_to_mod (index of a PARAMETER, call ptask_set_PARAMETER(idTask,
 * value, ...) */
int param_modification(int id_param_to_mod, int unit) {

    int i, value;
    char *endptr;

    i = strtol(index_modification, &endptr, 10);
    value = strtol(value_modification, &endptr, 10);

    if (i > NUM_T_TEST) {
        alert(" error index task ", NULL, NULL, "OK", NULL, 0, 0);
        return -1;
    }

    switch (id_param_to_mod) {

    case PARAM_PERIOD:
        printf("TASK %d	set period value:%d\n", i, value);
        ptask_set_period(i, value, unit);
        break;

    case PARAM_DEADLINE:
        printf("TASK %d	set deadline value:%d\n", i, value);
        ptask_set_deadline(i, value, unit);
        break;

    case PARAM_CPU:
        printf("TASK %d	set cpu value:%d\n", i, value);
        ptask_migrate_to(i, value);
        break;

    case PARAM_PRIORITY:
        printf("TASK %d	set priority value:%d\n", i, value);
        ptask_set_priority(i, value);
        break;
    }
    return i;
}

/* It draws task's parameters */
void draw_param_task(int i, int unit) {

    int spazio_colonna = 112;
    int h_riga = 10;
    int X_partenza = XMIN + 15;
    textprintf_ex(screen, font, X_partenza, BASE1 + 2 * h_riga + h_riga * i,
                  FGC, BGC, "%d", i);
    textprintf_ex(screen, font, X_partenza + spazio_colonna,
                  BASE1 + 2 * h_riga + h_riga * i, FGC, BGC, "%d     ",
                  ptask_get_period(i, unit));
    textprintf_ex(screen, font, X_partenza + spazio_colonna * 2,
                  BASE1 + 2 * h_riga + h_riga * i, FGC, BGC, "%d     ",
                  ptask_get_deadline(i, unit));
    textprintf_ex(screen, font, X_partenza + spazio_colonna * 3,
                  BASE1 + 2 * h_riga + h_riga * i, FGC, BGC, "%d",
                  ptask_get_processor(i));
    textprintf_ex(screen, font, X_partenza + spazio_colonna * 4,
                  BASE1 + 2 * h_riga + h_riga * i, FGC, BGC, "%d",
                  ptask_get_priority(i));
}

/*--------------------------------------------------------------*/
/*			MAIN process
 */
/*--------------------------------------------------------------*/

int main(void) {
    int i, k;
    int c; /* character from keyboard			*/
    int ret = 0;
    int tipo_prio = PRIO_DIFF; /* System Information */
    int sched = SCHED_OTHER;
    int part = PARTITIONED; // PARTITIONED;
    int prot = NO_PROTOCOL;
    int unit = scale;
    int id_param_to_mod;
    bool act_deferred = false, act_measure = false;
    tpars tp_temp;

    /* init ALLEGRO and semaphores */
    init();

    /* test a function for initializing a structure tpars with default
     * parameters */
    ptask_param_init(tp_temp);

    printf_param(tp_temp, "inizializzata da ptask_param_init(tpars tp)", unit);

    /******		READINGS PARAMETERS		*****/

    /* reading parameters from file */
    get_data();

    if (num_tasks > NUM_T_TEST) {
        fprintf(stderr, "Errore!!! non puoi superare i %d task!...modifica il "
                        "file .dat o la define.\n",
                NUM_T_TEST);
        exit(-1);
    }

    /******		INIZIALIZATION OF SYSTEM	*****/
    ptask_init(sched, part, prot);

    /******		DISPLAY INFORMATION			*****/
    /*											   3 -> indicates the test 3AdvancedCreationTest
     */
    draw_system_info(sched, part, prot, tipo_prio, 3, false);

    /******	test of functions ptask_param_PARAMETER(tpars tp, PARAMETER, int
     * unit): set structure tp of task *****/

    tpars tp[num_tasks];

    const char *td = "99:";
    const char *str = "Struttura tpars del task";
    int len = strlen(td) + strlen(str) + 1;
    char string[len];

    for (i = 0; i < num_tasks; i++) {
        snprintf(string, len, "%s%d:", str, i);
        tp[i] = TASK_SPEC_DFL;
        ptask_param_init(tp[i]);
        ptask_param_period(tp[i], period[i], unit);
        ptask_param_deadline(tp[i], rel_dl[i], unit);
        ptask_param_priority(tp[i], prio[i]);
        ptask_param_processor(tp[i], processore[i]);
        ptask_param_activation(tp[i], act_flag[i]);

        if (measure_flag[i] == 1)
            ptask_param_measure(
                tp[i]); // sets to 1, so activates measure of exec time	*/
        ptask_param_argument(tp[i], &colors[i]);
        printf_param(tp[i], string, unit);
    }

    int tid[num_tasks];

    for (i = 0; i < num_tasks; i++) {

        if (dipPeriodo == 0)
            tid[i] = ptask_create_param(periodicBall_testParam, &tp[i]);
        else
            tid[i] = ptask_create_param(periodicBall_testParamDEP_PER, &tp[i]);

        if (act_flag[i] == NOW)
            state_task[i] = ATTIVO;

        printf("Creato (e a seconda del act:flag in tpars anche attivato) "
               "task%d\n",
               tid[i]);

        if (!act_deferred && (act_flag[i] == DEFERRED))
            act_deferred = true;
        if (!act_measure && (measure_flag[i] == 1))
            act_measure = true;
    }
    pthread_mutex_lock(&mxa);
    for (i = 0; i < num_tasks; i++)
        draw_param_task(tid[i], unit);
    pthread_mutex_unlock(&mxa);

    do {
        k = 0;
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }

        /* To modify dinamically parameters of tasks */
        if (k == KEY_SPACE) {

            /* Creates and show dialog to modify dinamically parameters of tasks
             * and return index of parameter to modify*/
            id_param_to_mod = modification_param();

            printf("si vuole modificare il parametro %d\n", id_param_to_mod);

            /* from id_param_to_mod, call ptask_set_PARAMETER(idTask, value,
             * ...) */
            i = param_modification(id_param_to_mod, unit);

            /*update task's information on the screen */
            pthread_mutex_lock(&mxa);
            draw_param_task(i, unit);
            pthread_mutex_unlock(&mxa);
        }

        /* Activation mode of tasks is DEFERRED*/
        if (k == KEY_A && act_deferred) {

            act_deferred = false;

            for (i = 0; i < num_tasks; i++) {
                if (act_flag[i] == 0) {
                    ret = ptask_activate(i);
                    if (ret != -1) {
                        printf("Task %d activated\n", tid[i]);
                        state_task[i] = ATTIVO;
                    }
                }
            }
        }

        /* exit and printing of measure */
        if (k == KEY_M && act_measure) {

            act_measure = false;
            break;
        }

    } while (k != KEY_ESC);

    /**** PRINTING OF STATISTICAL MEASURES ****/

    printf("	*** If flag is setted, printing the stats (in microseconds) "
           "***\n");

    for (i = 0; i < num_tasks; i++) {
        if (state_task[i] != ATTIVO || (measure_flag[i] != 1))
            continue;
        tspec wcet = ptask_get_wcet(i);
        tspec acet = ptask_get_avg(i);
        tspec total_exec = ptask_get_total(i); // total execution time performed
                                               // by all the jobs since the task
                                               // creation time.
        printf("TASK %d: WCET = %ld\t ACET = %ld\t NINST=%d\t TOTEXEC=%ld\n",
               tid[i], tspec_to(&wcet, MICRO), tspec_to(&acet, MICRO),
               ptask_get_numinstances(i), tspec_to(&total_exec, MICRO));
    }
    printf("*** End of statistics ***\n\n");

    /*** PRINTING OF NEXT ACTIVATION TIME AND START TIME OF THE TASKS (first
     * MAX_SAMPLE sample) ***/
    printf("*** Printing first %d activation_time/start_time(ms) of the tasks "
           "***\n",
           MAX_SAMPLE);
    for (i = 0; i < num_tasks; i++) {

        if (state_task[i] != ATTIVO)
            continue;

        printf("TASK %d:\n", i);
        for (k = 1; k < MIN(MAX_SAMPLE, sample[i]); k++) {

            printf("\tnext_at[%d]-next_at[%d] = "
                   "%ld\tstart_time[%d]-start_time[%d] = %ld\n",
                   k, k - 1, v_next_at[i][k] - v_next_at[i][k - 1], k, k - 1,
                   start_time[i][k] - start_time[i][k - 1]);
        }
        printf("\n");
    }
    printf("*** End of activation time and start time ***\n");

    allegro_exit();
    return 0;
}
END_OF_MAIN()

/*--------------------------------------------------------------*/
