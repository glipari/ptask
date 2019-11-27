
#include "graphics.h"

const char *string_sched[] = {"SCHEDULER = OTHER", "SCHEDULER = FIFO",
                              "SCHEDULER = RR"};
const char *string_prior[] = {"PRIORITY  = ALL EQUAL", "PRIORITY  = ALL DIFF.",
                              "PRIORITY  = CUSTOM"};
const char *string_part[] = {" -PARTITIONING = PARTITIONED- ",
                             "-- PARTITIONING = GLOBAL --"};
const char *string_prot[] = {" PROTOCOL = PRIO_INHERITANCE",
                             " PROTOCOL = PRIO_CEILING",
                             " PROTOCOL = NO_PROTOCOL"};

#define MAX_STR_PROT strlen(string_prot[0])
#define MAX_STR_PART strlen(string_part[0])

/* s = scheduler, p = task's priority , tipo_test={0:SCHED,1:PART,2:PROT,
 * 3:PARAM_TEST, 5:MODE_TEST}*/
void draw_system_info(int s, int part, int prot, int p, int tipo_test,
                      bool verbose) {

    int len_stringProt_Part = MAX_STR_PROT + MAX_STR_PART + 1;
    int x, nc;
    int col_prot, col_sched, col_part; // colors of various modes
    int col_selected = 14;             // selected color = YELLOW
    col_prot = col_sched = col_part = FGC;

    if (tipo_test == SCHED)
        col_sched = col_selected;
    else if (tipo_test == PART)
        col_part = col_selected;
    else if (tipo_test == PROT)
        col_prot = col_selected;
    else
        col_prot = 15;

    clear_to_color(screen, BGC);

    /*rectangle for animation*/
    rect(screen, XMIN - L - 1, BASE - 1, XMAX + L + 1, TOP + L + 1, FGC);

    /*rectangle for tasks informations*/
    rect(screen, XMIN - L - 1, BASE1 - 1, XMAX + L + 1, TOP1 + L + 10 + 1, FGC);

    /*draw system information  ( protocol - partitioning - scheduler )*/
    textprintf_ex(screen, font, 2, 10, col_prot, BGC, "%s", string_prot[prot]);

    textprintf_ex(screen, font, 2 + PIXEL_CHAR * MAX_STR_PROT, 10, col_part,
                  BGC, "%s", string_part[part]);

    x = len_stringProt_Part * PIXEL_CHAR;
    textout_ex(screen, font, string_sched[s], x, 10, col_sched, 0);

    nc = ptask_getnumcores();
    textprintf_ex(screen, font, 2, 25, FGC, BGC, " NumCores = %d", nc);

    textout_ex(screen, font, string_prior[p], x, 25, FGC, 0);

    if (tipo_test <= PROT)
        textout_ex(screen, font, "ACTIVATION  SEQUENCE = ", XMIN, BASE1 + 10,
                   FGC, 0);
    else if (tipo_test == 3) {
        textout_ex(screen, font, "  id Task  ", XMIN, BASE1 + 10, FGC, 0);
        textout_ex(screen, font, "  period  ", XMIN + 112, BASE1 + 10, FGC, 0);
        textout_ex(screen, font, "  deadline  ", XMIN + 112 * 2, BASE1 + 10,
                   FGC, 0);
        textout_ex(screen, font, "  cpu id  ", XMIN + 112 * 3, BASE1 + 10, FGC,
                   0);
        textout_ex(screen, font, "  priority  ", XMIN + 112 * 4, BASE1 + 10,
                   FGC, 0);
    } else
        textout_ex(screen, font, " --->  MODE  ACTIVE  = ", XMIN, BASE1 + 10,
                   FGC, 0);

    if (verbose) {
        textout_ex(screen, font, "            PRIORITY = ", XMIN, BASE1 + 20,
                   FGC, 0);
        textout_ex(screen, font, "TERMINATION SEQUENCE = ", XMIN, BASE1 + 30,
                   FGC, 0);
    }

    if (tipo_test == 5) { // checking for modeTest
        textout_ex(screen, font, "KEY [ Z ] to active DEFAULT mode", 5,
                   YWIN - 40, 10, 0);
        textout_ex(screen, font, "KEY [ X ] to active MODE_A", 5, YWIN - 30, 10,
                   0);
        textout_ex(screen, font, "KEY [ C ] to active MODE_B", 5, YWIN - 20, 10,
                   0);
    } else if (tipo_test == 3) {
        textout_ex(screen, font,
                   "KEY [ A ] to activate tasks whit act_flag = 0", 5,
                   YWIN - 20, 10, 0);
        textout_ex(screen, font,
                   "KEY [ M ] to terminate tasks and show measure ", 5,
                   YWIN - 30, 10, 0);

    } else
        textout_ex(screen, font, "KEY [1-9] to activate tasks", 5, YWIN - 20,
                   10, 0);

    textout_ex(screen, font, "ESC exit", XWIN - 70, YWIN - 20, 12, 0);
}

/* draw the identifier of the task activated */
void draw_activation(int numTActive, int idT, int prio, bool verbose) {

    char str_prio[3];
    char str_id[3];
    int x;
    snprintf(str_id, 3, "T%d", idT + 1);
    snprintf(str_prio, 3, "%d", prio);

    x = XMIN + ((strlen("ACTIVATION  SEQUENCE = ") + 1) * PIXEL_CHAR) +
        (3 * numTActive) * PIXEL_CHAR;
    pthread_mutex_lock(&mxa);
    textout_ex(screen, font, str_id, x, BASE1 + 10, FGC, 0);
    if (verbose)
        textout_ex(screen, font, str_prio, x, BASE1 + 20, FGC, 0);
    pthread_mutex_unlock(&mxa);
}

/*type= {PRIO_EQUAL = 0: task with equal priority,
 * 		 PRIO_DIFF  = 1: task with different priority,
 * 		 PRIO_CUSTOM= 2: task with customizable priority} */
void init_vettore_prio(int tipo, int *prio, int dim) {
    int prioMAX = PRIO;
    int i = 0;

    if (tipo == 0) {
        for (i = 0; i < dim; i++) {
            prio[i] = prioMAX;
        }
    } else if (tipo == 1) {
        for (i = 0; i < dim; i++) {
            prio[i] = prioMAX - i;
        }
    }
}

char *listboxModification_getter(int index, int *list_size) {
    static char *strings[] = {"period", "deadline", "cpu", "priority"};
    if (index < 0) {
        *list_size = 4;
        return NULL;
    } else
        return strings[index];
}

/* Dialog used to modify dinamically parameters of tasks */
DIALOG box_modification[] = {
    /* (dialog proc) (x) (y) (w) (h) (fg) (bg) (key) (flags) (d1) (d2) (dp)
       (dp2) (dp3) */

    {d_box_proc, XMIN, TOP1 - 30, 560, 40, FGC, BGC, 0, 0, 0, 0, NULL, NULL,
     NULL},

    {d_text_proc, XMIN + 10, TOP1 - 27, 50, 20, FGC, BGC, 0, 0, 0, 0,
     "index task", NULL, NULL},
    {d_edit_proc, XMIN + 35, TOP1 - 12, 18, 20, 0, 15, 0, 0, 2, 0,
     index_modification, NULL, NULL},

    {d_text_proc, XMIN + 170, TOP1 - 27, 50, 20, FGC, BGC, 0, 0, 0, 0,
     "select param", NULL, NULL},
    {d_list_proc, XMIN + 170, TOP1 - 15, 100, 20, FGC, 0, 0, 0, 0, 0,
     listboxModification_getter, NULL, NULL},

    {d_text_proc, XMIN + 340, TOP1 - 27, 50, 20, FGC, BGC, 0, 0, 0, 0,
     "insert value", NULL, NULL},
    {d_edit_proc, XMIN + 360, TOP1 - 12, 50, 20, 0, 15, 0, 0, 9, 0,
     value_modification, NULL, NULL},

    {d_button_proc, XMAX - 50, TOP1 - 22, 40, 25, 10, 0, 0, D_EXIT, 0, 0, "OK",
     NULL, NULL},

    /* Final object */
    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}};

/* Creates and show dialog to modify dinamically parameters of tasks
* and return index of parameter to modify*/
int modification_param() {

    do_dialog(box_modification, -1);

    rectfill(screen, XMIN, TOP1 + 10, XMIN + 560, TOP1 - 30, 0);

    return box_modification[4].d1;
}
