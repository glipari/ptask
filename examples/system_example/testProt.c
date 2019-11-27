#define _GNU_SOURCE
#include "graphics.h"
#include "testParameters.h"
#include <sched.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define COLOR_SEZ_A 12
#define COLOR_SEZ_B 10
#define COLOR_NORM_EXE 9
#define SEZ_A 0
#define SEZ_B 1

int prtime[MAX_TASKS];   /* number of cycles	*/
int period[MAX_TASKS];   /* task periods		*/
int dline[MAX_TASKS];    /* task deadline	*/
int prio[MAX_TASKS];     /* task priority	*/
int offset[MAX_TASKS];   /* offset start		*/
int resource[MAX_TASKS]; /* resource utilized  rA=0 Rb=1 */

int num_tasks;     /* number of tasks	*/
float scale = 1.0; /* time scale		*/
long t_start;

/*--------------------------------------------------------------*/
/*  Reads task parameters from a configuration file     	*/
/*--------------------------------------------------------------*/

void get_data() {
    char c;
    int i;
    FILE *fp;

    fp = fopen("protocol.dat", "r");
    if (fp == NULL) {
        printf("File schedSezCritic.dat not found\n");
        exit(1);
    }

    do {
        c = getc(fp);
    } while (c != ':');
    fscanf(fp, "%f", &scale);

    i = 1;
    do {
        do {
            c = getc(fp);
        } while ((c != ':') && (c != EOF));
        if (c != EOF) {
            fscanf(fp, "%d", &prtime[i]);
            fscanf(fp, "%d", &period[i]);
            fscanf(fp, "%d", &dline[i]);
            fscanf(fp, "%d", &prio[i]);
            fscanf(fp, "%d", &offset[i]);
            fscanf(fp, "%d", &resource[i]);
            i++;
        }
    } while (c != EOF);

    num_tasks = i;
    fclose(fp);
}

/*--------------------------------------------------------------*/
/*  Prints a grid to show task periods during execution		*/
/*--------------------------------------------------------------*/

void print_grid(int prot) {
    int i, x, y, k;
    int lev;
    int at, dl;
    char buf[100];

    /*------------------------------------------------------*/
    /* Draw header						*/
    /*------------------------------------------------------*/

    clear_to_color(screen, BGC);
    rect(screen, XMIN_GRID + 30, YMIN_GRID + 1, XMAX_GRID - 30, 40, 4);
    if (prot == PRIO_INHERITANCE)
        sprintf(buf, "Protocol: PRIO_INHERITANCE");
    if (prot == PRIO_CEILING) {
        sprintf(buf, "Protocol: PRIO_CEILING");
    }
    if (prot == NO_PROTOCOL)
        sprintf(buf, "Protocol: NO_PROTOCOL");

    textout_centre_ex(screen, font, buf, 320, 10, 14, 0);
    textout_ex(screen, font, "Normal Exe", XMIN_GRID + 40, 30, COLOR_NORM_EXE,
               BGC);
    textout_ex(screen, font, "resource A", XMIN_GRID + 40 + 100, 30,
               COLOR_SEZ_A, BGC);
    textout_ex(screen, font, "resource B", XMIN_GRID + 40 + 200, 30,
               COLOR_SEZ_B, BGC);

    textout_centre_ex(screen, font, "(ESC to exit)", 320, YWIN - 20, 3, 0);
    sprintf(buf, "Time scale: %4.1f", scale);
    textout_ex(screen, font, buf, XMIN_GRID + 40, 10, 7, 0);

    /*------------------------------------------------------*/
    /* Draw grid						*/
    /*------------------------------------------------------*/

    for (x = OFFSET; x < XMAX_GRID; x += DXGRID / scale)
        for (y = LEV0; y < LEV0 + DLEV * (num_tasks - 1); y += DYGRID)
            putpixel(screen, x, y, GRIDCOL);

    /*------------------------------------------------------*/
    /* Draw task timelines					*/
    /*------------------------------------------------------*/

    for (i = 1; i < num_tasks; i++) {

        lev = LEV0 + DLEV * i;
        at = OFFSET;

        sprintf(buf, "T%d", i);
        textout_ex(screen, font, buf, 10, lev - 8, TLCOL, 0);
        line(screen, OFFSET, lev, XMAX_GRID, lev, TLCOL);

        k = 0;
        do {
            at = k * period[i];
            x = OFFSET + at / scale;
            if (x < XMAX_GRID)
                line(screen, x, lev, x, lev - 20, TLCOL);

            dl = at + dline[i];
            x = OFFSET + dl / scale;
            if (x < XMAX_GRID) {
                line(screen, x, lev - 1, x, lev - 15, DLCOL);
                line(screen, x, lev - 1, x - 1, lev - 5, DLCOL);
                line(screen, x, lev - 1, x + 1, lev - 5, DLCOL);
            }
            k++;

        } while (x < XMAX_GRID);
    }
}

/*--------------------------------------------------------------*/

void init() {

    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    install_mouse();
    srand(time(NULL));
}

void set_sem_sezC(int protocol) {

    pthread_mutex_init(&mx_sezNorm, 0);
    printf("Selected protocol : -%d-  \n", protocol);

    if (protocol == PRIO_INHERITANCE) {
        printf("PRIO_INHERITANCE  \n");
        pmux_create_pi(&mx_sezA);
        pmux_create_pi(&mx_sezB);

    } else if (protocol == PRIO_CEILING) {
        printf("PRIO_CEILING   \n");
        pmux_create_pc(&mx_sezA, PRIO);
        pmux_create_pc(&mx_sezB, PRIO);

    } else {

        pthread_mutex_init(&mx_sezA, 0);
        pthread_mutex_init(&mx_sezB, 0);
    }
}

/*--------------------------------------------------------------*/
/*  GENERIC PERIODIC TASK					*/
/*--------------------------------------------------------------*/

void task() {
    int x = 0;
    long t = 0;
    int i, k;
    int lev1, lev2, col;

    i = ptask_get_index();

    lev1 = LEV0 + DLEV * i - 2;
    lev2 = LEV0 + DLEV * i - 2 - DEX;
    col = COLOR_NORM_EXE;

    while (x < XWIN) {

        col = COLOR_NORM_EXE;
        k = 0;
        while (k <= (prtime[i] / 2)) {
            t = ptask_gettime(MILLI);
            x = OFFSET + (t - t_start) / scale;
            pthread_mutex_lock(&mx_sezNorm);
            line(screen, x, lev1, x, lev2, col);
            pthread_mutex_unlock(&mx_sezNorm);
            while (ptask_gettime(MILLI) == t)
                ;
            k++;
        }

        if (resource[i] == SEZ_A) {
            pthread_mutex_lock(&mx_sezA);
            col = COLOR_SEZ_A;
        }
        if (resource[i] == SEZ_B) {
            pthread_mutex_lock(&mx_sezB);
            col = COLOR_SEZ_B;
        }

        while (k <= prtime[i]) {

            t = ptask_gettime(MILLI);
            x = OFFSET + (t - t_start) / scale;
            line(screen, x, lev1, x, lev2, col);
            while (ptask_gettime(MILLI) == t)
                ;
            k++;
        }

        if (resource[i] == SEZ_A)
            pthread_mutex_unlock(&mx_sezA);

        if (resource[i] == SEZ_B)
            pthread_mutex_unlock(&mx_sezB);

        ptask_wait_for_period();
    }

    printf("   T%d terminated!\n", i);
}

/*--------------------------------------------------------------*/
/*  CREATOR TASK						*/
/*--------------------------------------------------------------*/

void gen() {
    int i, j;

    for (i = 1; i < num_tasks; i++) {
        tpars params = TASK_SPEC_DFL;
        params.period = tspec_from(period[i], MILLI);
        params.rdline = tspec_from(dline[i], MILLI);
        params.priority = prio[i];
        params.act_flag = DEFERRED;
        params.processor = 0;
        j = ptask_create_param(task, &params);
        if (j < 0)
            ptask_syserror("gen()", "error in creating task");
        ptask_activate_at(j, offset[i], MILLI);
    }
}

/*--------------------------------------------------------------*/
/*			MAIN process
 */
/*--------------------------------------------------------------*/

int main(void) {

    int ret = 0, nc;
    int c;
    int key = 0;
    int part = PARTITIONED; // PARTITIONED, GLOBAL
    int sched = SCHED_FIFO;

    get_data();
    init();
    ret = select_prot();
    if (ret == -1) {
        allegro_exit();
        return 0;
    }

    print_grid(ret);
    ptask_init(sched, part, ret);
    t_start = ptask_gettime(MILLI);
    set_sem_sezC(ret);

    nc = ptask_getnumcores();
    textprintf_ex(screen, font, 480, 10, 7, BGC, "(NumCores = %d)", nc);

    int gen_id = ptask_create_prio(gen, 100, 30, NOW);
    if (gen_id < 0) {
        printf("Could not create task gen\n");
        exit(-1);
    }

    while (key != KEY_ESC) {

        if (keypressed()) {
            c = readkey();
            key = c >> 8;
        }
    }

    pmux_destroy(&mx_sezNorm);
    pmux_destroy(&mx_sezA);
    pmux_destroy(&mx_sezB);

    allegro_exit();
    return 0;
}

/*--------------------------------------------------------------*/
