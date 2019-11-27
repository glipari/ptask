#include "graphics.h"

const char *stringa_sched[] = {"SCHEDULER = OTHER", "SCHEDULER = FIFO",
                               "SCHEDULER = RR"};
const char *stringa_prior[] = {"PRIORITY  = ALL EQUAL", "PRIORITY  = ALL DIFF.",
                               "PRIORITY  = CUSTOM"};
const char *stringa_part[] = {" -PARTITIONING = PARTITIONED- ",
                              "-- PARTITIONING = GLOBAL --"};
const char *stringa_prot[] = {" PROTOCOL = PRIO_INHERITANCE",
                              " PROTOCOL = PRIO_CEILING",
                              " PROTOCOL = NO_PROTOCOL"};

#define MAX_STR_PROT strlen(stringa_prot[0])
#define MAX_STR_PART strlen(stringa_part[0])

/* s = scheduler, p = task's priority , tipo_test={0:SCHED,1:PART,2:PROT}*/
void draw_system_info(int s, int part, int prot, int p, int tipo_test) {

    int len_stringProt_Part = MAX_STR_PROT + MAX_STR_PART + 1;
    int x, nc;
    int col_prot, col_sched;
    int col_part = 14; // yellow
    col_prot = col_sched = FGC;
    x = len_stringProt_Part * PIXEL_CHAR;
    nc = ptask_getnumcores(); // # of core

    clear_to_color(screen, BGC);

    /*rectangle for animation*/
    rect(screen, XMIN - L - 1, BASE - 1, XMAX + L + 1, TOP + L + 1, FGC);
    /*rectangle for tasks informations*/
    rect(screen, XMIN - L - 1, BASE1 - 1, XMAX + L + 1, TOP1 + L + 10 + 1, FGC);

    /*draw system information  ( protocol - partitioning - scheduler )*/
    textprintf_ex(screen, font, 2, 10, col_prot, BGC, "%s", stringa_prot[prot]);
    textprintf_ex(screen, font, 2 + PIXEL_CHAR * MAX_STR_PROT, 10, col_part,
                  BGC, "%s", stringa_part[part]);
    textout_ex(screen, font, stringa_sched[s], x, 10, col_sched, 0);

    textprintf_ex(screen, font, 2, 25, FGC, BGC, " NumCores = %d", nc);
    textout_ex(screen, font, stringa_prior[p], x, 25, FGC,
               0); // type of priority task

    textout_ex(screen, font, "ACTIVATION  SEQUENCE = ", XMIN, BASE1 + 10, FGC,
               0);
    textout_ex(screen, font, "KEY [1-9] to activate tasks", 5, YWIN - 20, 10,
               0);
    textout_ex(screen, font, "ESC exit", XWIN - 70, YWIN - 20, 12, 0);
}

// draw the identifier of the task activated
void draw_activation(int numTActive, int idT, int prio, bool verbose) {
    int x;

    x = XMIN + (24 * 8) + (3 * numTActive) * 8;
    pthread_mutex_lock(&mxa);
    textprintf_ex(screen, font, x, BASE1 + 10, FGC, BGC, "T%d", idT + 1);
    if (verbose)
        textprintf_ex(screen, font, x, BASE1 + 10, FGC, BGC, "%d", prio);
    pthread_mutex_unlock(&mxa);
}

char *listboxSched_getter(int index, int *list_size) {
    static char *strings[] = {"SCHED_OTHER", "SCHED_FIFO", "SCHED_RR"};
    if (index < 0) {
        *list_size = 3;
        return NULL;
    } else
        return strings[index];
}

char *listboxPart_getter(int index, int *list_size) {
    static char *strings[] = {"PARTITIONED", "GLOBAL"};
    if (index < 0) {
        *list_size = 2;
        return NULL;
    } else
        return strings[index];
}

char *listboxProt_getter(int index, int *list_size) {
    static char *strings[] = {"PRIO_INHERITANCE", "PRIO_CEILING",
                              "NO_PROTOCOL"};
    if (index < 0) {
        *list_size = 3;
        return NULL;
    } else
        return strings[index];
}

void init_vett(int tipo, int *vett, int dim, int val) {
    int i;

    if (tipo == INIZIALIZED) {
        for (i = 0; i < dim; i++) {
            vett[i] = val;
        }
    } else if (tipo == DIFFERENT) {
        for (i = 0; i < dim; i++) {
            vett[i] = val - i;
        }
    }
}

/*type= {PRIO_EQUAL = 0: task with equal priority,
 * 		 PRIO_DIFF  = 1: task with different priority,
 * 		 PRIO_CUSTOM= 2: task with customizable priority}*/
void init_vettore_prio(int type, int *prio, int dim) {
    int prioMAX = PRIO;
    init_vett(type, prio, dim, prioMAX);
}

/* Dialog used to select scheduler*/
DIALOG sched_dialog[8] = {
    /* object structure */
    /* (dialog proc) (x) (y) (w) (h) (fg) (bg) (key) (flags) (d1) (d2) (dp)
       (dp2) (dp3) */
    {d_clear_proc, 0, 0, 0, 0, FGC, BGC, 0, 0, 0, 0, NULL, NULL, NULL},
    {d_ctext_proc, XWIN / 2 - 150, 30, 300, 50, 14, BGC, 0, 0, 0, 0,
     "----- SCHEDULER TEST -----", NULL, NULL},
    {d_box_proc, 20, YWIN / 2 - 80, 600, 50, FGC, BGC, 0, 0, 0, 0, NULL, NULL,
     NULL},
    {d_text_proc, XMIN, YWIN / 2 - 60, 50, 20, FGC, BGC, 0, 0, 0, 0,
     "Scheduler setting", NULL, NULL},
    {d_list_proc, XWIN / 2 + 50, YWIN / 2 - 70, 150, 30, FGC, 0, 0, 0, 0, 0,
     listboxSched_getter, NULL, NULL},
    {d_button_proc, 80, YWIN - 80, 161, 49, 10, 0, 0, D_EXIT, 0, 0, "OK", NULL,
     NULL},
    {d_button_proc, XWIN - 280, YWIN - 80, 161, 49, 12, 0, 0, D_EXIT, 0, 0,
     "EXIT", NULL, NULL},
    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}};

/* Dialog used to select protocol*/
DIALOG prot_dialog[] = {

    {d_clear_proc, 0, 0, 0, 0, FGC, BGC, 0, 0, 0, 0, NULL, NULL, NULL},
    {d_ctext_proc, XWIN / 2 - 150, 30, 300, 50, 14, BGC, 0, 0, 0, 0,
     "----- PROTOCOL TEST -----", NULL, NULL},
    {d_box_proc, 20, YWIN / 2 - 80, 600, 50, FGC, BGC, 0, 0, 0, 0, NULL, NULL,
     NULL},
    {d_text_proc, XMIN, YWIN / 2 - 60, 50, 20, FGC, BGC, 0, 0, 0, 0,
     "Protocol setting", NULL, NULL},
    {d_list_proc, XWIN / 2 + 50, YWIN / 2 - 70, 150, 30, FGC, 0, 0, 0, 0, 0,
     listboxProt_getter, NULL, NULL},
    {d_button_proc, 80, YWIN - 80, 161, 49, 10, 0, 0, D_EXIT, 0, 0, "OK", NULL,
     NULL},
    {d_button_proc, XWIN - 280, YWIN - 80, 161, 49, 12, 0, 0, D_EXIT, 0, 0,
     "EXIT", NULL, NULL},
    {d_box_proc, 20, YWIN / 2 + 30, 600, 50, FGC, BGC, 0, D_HIDDEN, 0, 0, NULL,
     NULL, NULL},
    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}};

/* Dialog used to select type of partitioning*/
DIALOG part_dialog[] = {
    {d_clear_proc, 0, 0, 0, 0, FGC, BGC, 0, 0, 0, 0, NULL, NULL, NULL},
    {d_ctext_proc, XWIN / 2 - 150, 30, 300, 50, 14, BGC, 0, 0, 0, 0,
     "----- PARTITIONING TEST -----", NULL, NULL},
    {d_box_proc, 20, YWIN / 2 - 80, 600, 50, FGC, BGC, 0, 0, 0, 0, NULL, NULL,
     NULL},
    {d_text_proc, XMIN, YWIN / 2 - 60, 50, 20, FGC, BGC, 0, 0, 0, 0,
     "Partitioning setting", NULL, NULL},
    {d_list_proc, XWIN / 2 + 50, YWIN / 2 - 70, 150, 30, FGC, 0, 0, 0, 0, 0,
     listboxPart_getter, NULL, NULL},
    {d_button_proc, 80, YWIN - 80, 161, 49, 10, 0, 0, D_EXIT, 0, 0, "OK", NULL,
     NULL},
    {d_button_proc, XWIN - 280, YWIN - 80, 161, 49, 12, 0, 0, D_EXIT, 0, 0,
     "EXIT", NULL, NULL},
    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}};

DIALOG d_final = {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL};

bool error_edit(char *stringa, char *endptr, long int val) {
    char *error2 = "Error in conversion! long(strtol)!!!\n";
    char *error3_4 = "Error! entered letters or symbols!!!\n";
    bool errore_trovato = false;
    char *tmp2, *tmp3;

    tmp2 = tmp3 = NULL;
    errno = 0;

    /* controls for various types of errors for the input data*/
    bool cond_error1 = ((val == LONG_MAX) || (val == LONG_MIN));
    bool cond_error2 =
        (errno == ERANGE && cond_error1) || (errno != 0 && (val == 0));
    bool cond_error3 = (endptr == stringa);
    bool cond_error4 = (*endptr != '\0');

    if (cond_error1 || cond_error2 || cond_error3 || cond_error4) {
        errore_trovato = true;

        if (cond_error2) {
            perror(error2);
        }

        if (cond_error2 || cond_error3 || cond_error4) {
            tmp3 = error3_4;
            alert(tmp2, tmp3, NULL, "OK", NULL, 0, 0);
        }
    }
    if (!errore_trovato && ((val > PRIO) || (val < 0))) {
        alert("Error! entered high priority!!!", NULL, NULL, "OK", NULL, 0, 0);
        errore_trovato = true;
    }
    return errore_trovato;
}

/* Function to select system scheduler*/
int select_sched() {
    int ret = 0;

    ret = do_dialog(sched_dialog, -1);

    if (ret == 6 || ret < 0)
        return -1;
    else {
        return sched_dialog[4].d1;
    }
}

/* Function to select system protocol*/
int select_prot() {
    int ret = 0;

    ret = do_dialog(prot_dialog, -1);

    if (ret == 6 || ret < 0)
        return -1;
    else {
        return prot_dialog[4].d1;
    }
}

/* Function to select type of partitioning*/
int select_part() {

    int ret = 0;

    ret = do_dialog(part_dialog, -1);

    if (ret == 6 || ret < 0)
        return -1;
    else {
        return part_dialog[4].d1;
    }
}
