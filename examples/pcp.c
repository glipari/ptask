/*--------------------------------------------------------------*/
/*		TEST ON PRIORITY CEILING			*/
/*--------------------------------------------------------------*/
#define _GNU_SOURCE
#include "pmutex.h"
#include "ptask.h"
#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XMIN 0
#define YMIN 0
#define XMAX 640
#define YMAX 480
#define LEV0 80   /* Y Level of the MAIN timeline		*/
#define DLEV 50   /* Y space between timelines		*/
#define DEX 6     /* thickness of the execution		*/
#define DXGRID 20 /* horizontal space between grid lines	*/
#define DYGRID 5  /* vertical space between grid points	*/
#define GRIDCOL 7 /* color of the grid lines		*/
#define BGC 0     /* background color			*/
#define TLCOL 3   /* color for the timeline		*/
#define DLCOL 12  /* color for the deadlines		*/
#define MAINCOL 7 /* MAIN color				*/
#define CSCOLA 13 /* color for critical section A		*/
#define CSCOLB 14 /* color for critical section B		*/

#define OFFSET 30 /* initial offset			*/

#define NOP 0 /* No Protocol				*/
#define PIP 1 /* Priority Inheritance Protocol	*/
#define PCP 2 /* Priority Ceiling Protocol		*/

int prtime[MAX_TASKS]; /* number of cycles	*/
int csa[MAX_TASKS];    /* task periods		*/
int csb[MAX_TASKS];    /* task periods		*/
int period[MAX_TASKS]; /* task periods		*/
int dline[MAX_TASKS];  /* task deadline	*/
int prio[MAX_TASKS];   /* task priority	*/

int nt;            /* number of tasks	*/
float scale = 1.0; /* time scale		*/

/* mutual exclusion semaphores  */
pthread_mutex_t mxa, muxA, muxB;
// pthread_mutexattr_t	matt;

/*--------------------------------------------------------------*/
/*  Reads task parameters from a configuration file     	*/
/*--------------------------------------------------------------*/

void get_data() {
    char c;
    int i;
    FILE *fp;

    fp = fopen("pcp.dat", "r");
    if (fp == NULL) {
        printf("File PCP.DAT not found\n");
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
            fscanf(fp, "%d", &csa[i]);
            fscanf(fp, "%d", &csb[i]);
            fscanf(fp, "%d", &period[i]);
            fscanf(fp, "%d", &dline[i]);
            fscanf(fp, "%d", &prio[i]);
            i++;
        }
    } while (c != EOF);

    nt = i;
    fclose(fp);
}

/*--------------------------------------------------------------*/
/*  Prints a grid to show task periods during execution		*/
/*--------------------------------------------------------------*/

void print_grid(int policy, int prot) {
    int i, x, y, k;
    int lev;
    int at, dl;
    char buf[50];

    /*------------------------------------------------------*/
    /* Draw header						*/
    /*------------------------------------------------------*/

    clear_to_color(screen, BGC);
    rect(screen, XMIN + 30, YMIN + 1, XMAX - 30, 40, 4);

    if (policy == SCHED_FIFO)
        sprintf(buf, "TASK schedule produced by SCHED_FIFO");
    if (policy == SCHED_RR)
        sprintf(buf, "TASK schedule produced by SCHED_RR");
    if (policy == SCHED_OTHER)
        sprintf(buf, "TASK schedule produced by SCHED_OTHER");
    textout_centre_ex(screen, font, buf, 320, 10, 14, 0);

    if (prot == PIP)
        sprintf(buf, "+ PIP");
    if (prot == PCP)
        sprintf(buf, "+ PCP");
    if (prot != NOP)
        textout_centre_ex(screen, font, buf, 488, 10, 14, 0);

    textout_centre_ex(screen, font, "(ESC to exit)", 320, 30, 3, 0);
    sprintf(buf, "Time scale: %4.1f", scale);
    textout_ex(screen, font, buf, XMIN + 40, 30, 7, 0);

    /*------------------------------------------------------*/
    /* Draw grid						*/
    /*------------------------------------------------------*/

    for (x = OFFSET; x < XMAX; x += DXGRID / scale)
        for (y = LEV0; y < LEV0 + DLEV * (nt - 1); y += DYGRID)
            putpixel(screen, x, y, GRIDCOL);

    /*------------------------------------------------------*/
    /* Draw task timelines					*/
    /*------------------------------------------------------*/

    textout_ex(screen, font, "MAIN", 10, LEV0 + 5, MAINCOL, 0);
    line(screen, OFFSET, LEV0, XMAX, LEV0, TLCOL);
    line(screen, OFFSET, LEV0, OFFSET, LEV0 - 10, TLCOL);

    for (i = 1; i < nt; i++) {

        lev = LEV0 + DLEV * i;
        at = OFFSET;

        sprintf(buf, "T%d", i);
        textout_ex(screen, font, buf, 10, lev - 8, TLCOL, 0);
        line(screen, OFFSET, lev, XMAX, lev, TLCOL);

        k = 0;
        do {
            at = k * period[i];
            x = OFFSET + at / scale;
            if (x < XMAX)
                line(screen, x, lev, x, lev - 20, TLCOL);

            dl = at + dline[i];
            x = OFFSET + dl / scale;
            if (x < XMAX) {
                line(screen, x, lev - 1, x, lev - 15, DLCOL);
                line(screen, x, lev - 1, x - 1, lev - 5, DLCOL);
                line(screen, x, lev - 1, x + 1, lev - 5, DLCOL);
            }
            k++;

        } while (x < XMAX);
    }
}

/*--------------------------------------------------------------*/

void init(int policy, int prot) {
    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XMAX, YMAX, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    print_grid(policy, prot);

    if (prot == PIP)
        ptask_init(policy, GLOBAL, PRIO_INHERITANCE);
    else if (prot == PCP)
        ptask_init(policy, GLOBAL, PRIO_CEILING);
    else {
        allegro_exit();
        ptask_syserror("pcp.c", "Wrong protocol");
    }

    if (prot == PIP) {
        pmux_create_pi(&mxa);
        pmux_create_pi(&muxA);
        pmux_create_pi(&muxB);
    } else if (prot == PCP) {
        pmux_create_pc(&mxa, prio[1]);
        pmux_create_pc(&muxA, prio[1]);
        pmux_create_pc(&muxB, prio[1]);
    }
}

/*--------------------------------------------------------------*/
/*  GENERIC PERIODIC TASK					*/
/*--------------------------------------------------------------*/

void task() {
    int x = 0;
    long t;
    int i, k;
    int lev1, lev2, col;
    int at = 0;

    i = ptask_get_index();

    lev1 = LEV0 + DLEV * i - 2;
    lev2 = LEV0 + DLEV * i - 2 - DEX;
    col = 1 + i % 15;
    // wait_for_activation();

    while (x < 640L) {

        // dl = at + ptask_get_deadline(i, MILLI);

        for (k = 0; k < prtime[i]; k++) {
            t = ptask_gettime(MILLI);
            x = OFFSET + t / scale;
            pthread_mutex_lock(&mxa);
            line(screen, x, lev1, x, lev2, col);
            pthread_mutex_unlock(&mxa);
            while (ptask_gettime(MILLI) == t)
                ;
        }

        /*----------------------------------------------*/
        /* Critical section guarded by muxA		*/
        /*----------------------------------------------*/
        if ((i != 2) && (i != 4)) {
            pthread_mutex_lock(&muxA);
            for (k = 0; k < csa[i]; k++) {
                t = ptask_gettime(MILLI);
                x = OFFSET + t / scale;
                pthread_mutex_lock(&mxa);
                line(screen, x, lev1 - 1, x, lev2 + 1, CSCOLA);
                pthread_mutex_unlock(&mxa);
                while (ptask_gettime(MILLI) == t)
                    ;
            }
            pthread_mutex_unlock(&muxA);
        }
        /*----------------------------------------------*/
        if (i != 4) {
            for (k = 0; k < prtime[i]; k++) {
                t = ptask_gettime(MILLI);
                x = OFFSET + t / scale;
                pthread_mutex_lock(&mxa);
                line(screen, x, lev1, x, lev2, col);
                pthread_mutex_unlock(&mxa);
                while (ptask_gettime(MILLI) == t)
                    ;
            }
        }
        /*----------------------------------------------*/
        /* Critical section guarded by muxB		*/
        /*----------------------------------------------*/
        if (i != 2) {
            pthread_mutex_lock(&muxB);
            for (k = 0; k < csb[i]; k++) {
                t = ptask_gettime(MILLI);
                x = OFFSET + t / scale;
                pthread_mutex_lock(&mxa);
                line(screen, x, lev1 - 1, x, lev2 + 1, CSCOLB);
                pthread_mutex_unlock(&mxa);
                while (ptask_gettime(MILLI) == t)
                    ;
            }
            pthread_mutex_unlock(&muxB);
        }
        /*----------------------------------------------*/

        if (ptask_deadline_miss()) {
            // TO BE DONE
            // sprintf(s, "%d", task_dmiss(i));
            // textout_ex(screen, font, s, OFFSET+dl/scale-4, lev1+8, 7, 0);
        }

        ptask_wait_for_period();
        at = at + ptask_get_period(i, MILLI);
    }
}

/*--------------------------------------------------------------*/
/*  CREATOR TASK						*/
/*--------------------------------------------------------------*/

void gen() {
    int i, j;

    for (i = 1; i < nt; i++) {
        j = ptask_create_prio(task, period[i], prio[i], DEFERRED);
        if (j < 0)
            ptask_syserror("gen()", "error in creating task");
    }

    for (i = 1; i < nt; i++) {
        ptask_activate(i);
    }
}

/*--------------------------------------------------------------*/
/*  MAIN TASK							*/
/*--------------------------------------------------------------*/

int main(void) {
    int lev1, lev2;
    int c, key = 0;
    long x = 0;
    long t;

    get_data();

    init(SCHED_FIFO, PCP);

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

        if (x < 640L) {
            t = ptask_gettime(MILLI);
            x = OFFSET + t / scale;
            pthread_mutex_lock(&mxa);
            line(screen, x, lev1, x, lev2, MAINCOL);
            pthread_mutex_unlock(&mxa);
        }
    }

    pmux_destroy(&mxa);
    pmux_destroy(&muxA);
    pmux_destroy(&muxB);

    allegro_exit();

    return 0;
}

/*--------------------------------------------------------------*/
