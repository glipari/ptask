#define _GNU_SOURCE
#include "graphics.h"
#include "testParameters.h"
#include <sched.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int prtime[MAX_TASKS]; /* number of cycles	*/
int csa[MAX_TASKS];    /* task periods		*/
int csb[MAX_TASKS];    /* task periods		*/
int period[MAX_TASKS]; /* task periods		*/
int dline[MAX_TASKS];  /* task deadline	*/
int prio[MAX_TASKS];   /* task priority	*/
int tid[MAX_TASKS];    /* task id			*/

int num_tasks;     /* number of tasks	*/
float scale = 1.0; /* time scale		*/

/* mutual exclusion semaphores  */
pthread_mutex_t mxa;

/*--------------------------------------------------------------*/
/*  Reads task parameters from a configuration file     	*/
/*--------------------------------------------------------------*/

void get_data() {
    char c;
    int i;
    FILE *fp;

    fp = fopen("scheduler.dat", "r");
    if (fp == NULL) {
        printf("File SCHED.DAT not found\n");
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
            i++;
        }
    } while (c != EOF);

    num_tasks = i;
    fclose(fp);
}

/*--------------------------------------------------------------*/
/*  Prints a grid to show task periods during execution		*/
/*--------------------------------------------------------------*/

void print_grid(int policy) {
    int i, x, y, k;
    int lev;
    int at, dl;
    char buf[100];

    /*------------------------------------------------------*/
    /* Draw header						*/
    /*------------------------------------------------------*/

    clear_to_color(screen, BGC);
    rect(screen, XMIN_GRID + 30, YMIN_GRID + 1, XMAX_GRID - 30, 40, 4);
    if (policy == SCHED_FIFO)
        sprintf(buf, "TASK schedule produced by SCHED_FIFO");
    if (policy == SCHED_RR) {
        sprintf(buf, "TASK schedule produced by SCHED_RR");
        struct timespec q;
        sched_rr_get_interval(0, &q);
        printf("Q: %ld s, %ld ns\n", q.tv_sec, q.tv_nsec);
    }
    if (policy == SCHED_OTHER)
        sprintf(buf, "TASK schedule produced by SCHED_OTHER");

    textout_centre_ex(screen, font, buf, 320, 10, 14, 0);

    textout_centre_ex(screen, font, "(ESC to exit)", 320, 30, 3, 0);
    sprintf(buf, "Time scale: %4.1f", scale);
    textout_ex(screen, font, buf, XMIN_GRID + 40, 30, 7, 0);

    /*------------------------------------------------------*/
    /* Draw grid						*/
    /*------------------------------------------------------*/

    for (x = OFFSET; x < XMAX_GRID; x += DXGRID / scale)
        for (y = LEV0; y < LEV0 + DLEV * (num_tasks - 1); y += DYGRID)
            putpixel(screen, x, y, GRIDCOL);

    /*------------------------------------------------------*/
    /* Draw task timelines					*/
    /*------------------------------------------------------*/

    textout_ex(screen, font, "MAIN", 10, LEV0 + 5, MAINCOL, 0);
    line(screen, OFFSET, LEV0, XMAX_GRID, LEV0, TLCOL);
    line(screen, OFFSET, LEV0, OFFSET, LEV0 - 10, TLCOL);

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
/*  GENERIC PERIODIC TASK					*/
/*--------------------------------------------------------------*/

void task() {
    int x = 0;
    long t = 0;
    int i, k;
    int lev1, lev2, col;
    int at = 0;

    i = ptask_get_index();

    lev1 = LEV0 + DLEV * i - 2;
    lev2 = LEV0 + DLEV * i - 2 - DEX;
    col = 1 + i % 15;

    while (x < XWIN) {

        for (k = 0; k < prtime[i]; k++) {
            t = ptask_gettime(MILLI);
            x = OFFSET + t / scale;
            pthread_mutex_lock(&mxa);
            line(screen, x, lev1, x, lev2, col);
            pthread_mutex_unlock(&mxa);
            while (ptask_gettime(MILLI) == t)
                ;
        }

        ptask_wait_for_period();
        at = at + ptask_get_period(i, MILLI);
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
        params.act_flag = NOW;
        params.processor = 0;
        j = ptask_create_param(task, &params);
        if (j < 0)
            ptask_syserror("gen()", "error in creating task");
    }
}

void init() {

    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    install_mouse();
    srand(time(NULL));
    pmux_create_pi(&mxa);
}

/*--------------------------------------------------------------*/
/*			MAIN process
 */
/*--------------------------------------------------------------*/

int main(void) {

    int ret = 0, nc;
    long x = 0;
    long t;
    int c;
    int key = 0;
    int lev1, lev2;
    int part = PARTITIONED;      // PARTITIONED, GLOBAL
    int prot = PRIO_INHERITANCE; // NO_PROTOCOL;

    get_data();
    init();
    ret = select_sched();
    if (ret == -1) {
        allegro_exit();
        return 0;
    }

    print_grid(ret);
    ptask_init(ret, part, prot);

    nc = ptask_getnumcores();
    textprintf_ex(screen, font, 480, 30, 7, BGC, "(NumCores = %d)", nc);

    lev1 = LEV0 - 2;
    lev2 = LEV0 - 2 - DEX;

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

        if (x < XWIN) {
            t = ptask_gettime(MILLI);
            x = OFFSET + t / scale;
            pthread_mutex_lock(&mxa);
            line(screen, x, lev1, x, lev2, MAINCOL);
            pthread_mutex_unlock(&mxa);
            while (ptask_gettime(MILLI) == t)
                ;
        }
    }

    pmux_destroy(&mxa);
    allegro_exit();
    return 0;
}
