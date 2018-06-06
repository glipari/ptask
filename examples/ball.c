/*--------------------------------------------------------------*/
/*		SIMULATION OF JUMPING BALLS			*/
/*	BALLC - uso della create con attributes			*/
/*--------------------------------------------------------------*/

#include "pmutex.h"
#include "ptask.h"
#include "tstat.h"
#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XWIN 640
#define YWIN 480
#define PI 3.1415
#define BGC 0    /* background color		*/
#define L 4      /* dimension of a ball		*/
#define G 9.8    /* acceleration of gravity	*/
#define BASE 40  /* position of the floor	*/
#define TOP 400  /* initial height of the ball	*/
#define XMIN 40  /* min position X of the ball	*/
#define XMAX 600 /* max position Y of the ball	*/
#define VELX 15. /* horizontal ball velocity	*/
#define VMIN 11. /* velocità minima per suono    */
#define PER 20   /* task period in ms		*/
#define DREL 20  /* realtive deadline in ms	*/
#define PRIO 80  /* task priority		*/

float v0[MAX_TASKS]; /* impact velocity with floor   */

/* mutual exclusion semaphores  */
pthread_mutex_t mxa; // = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mxv; // = PTHREAD_MUTEX_INITIALIZER;

int end = 0;

/*--------------------------------------------------------------*/
void init() {
    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    srand(time(NULL));

    rect(screen, XMIN - L - 1, BASE - 1, XMAX + L + 1, TOP + BASE + L + 1, 14);
    textout_centre_ex(screen, font, "SPACE to create a fly", XWIN / 2, YWIN / 2,
                      14, 0);
    textout_centre_ex(screen, font, "ESC exit", XWIN / 2, YWIN / 2 + 30, 14, 0);

    pmux_create_pi(&mxa);
    pmux_create_pi(&mxv);

    ptask_init(SCHED_FIFO, GLOBAL, PRIO_INHERITANCE);
}

/*--------------------------------------------------------------*/

void draw_ball(int x, int y, int c) { circlefill(screen, x, y, L, c); }

/*--------------------------------------------------------------*/
/*	Periodic task for ball simulation			*/
/*--------------------------------------------------------------*/

void palla() {
    int i, col, dcol = 0; /* indice del task  */
    int x, y;             /* coordinate grafiche pallina  */
    int ox, oy;           /* vecchia posizione pallina    */
    int x0;               /* posizione iniziale X pallina */
    float vx, vy;         /* velocità della pallina   */
    float t, tx;          /* variabile temporale      */
    // float	z, z0;		/* posizione della pallina  */
    float dt; /* incremento temporale     */
    double a; /* variabile di appoggio    */
    // long	j, k;

    i = ptask_get_index();
    col = 2 + i % 14;
    y = oy = (TOP - L);
    x = ox = x0 = XMIN;

    a = 2. * G * (float)(TOP - L);
    vy = sqrt(a);
    vx = VELX;
    tx = 0.0;
    t = vy / G;
    dt = ptask_get_period(i, MILLI) / 100.;

    // removed, unless a specific activation from the main is done
    // wait_for_activation();

    while (1) {
        x = x0 + vx * tx;
        y = BASE + vy * t - .5 * G * t * t;
        if (y < BASE) {
            t = 0.0;
            pthread_mutex_lock(&mxv);
            v0[i] = .9 * v0[i];
            vy = v0[i];
            pthread_mutex_unlock(&mxv);
            y = BASE + vy * t - .5 * G * t * t;
        }
        if (x > XMAX) {
            tx = 0.0;
            x0 = XMAX;
            vx = -vx;
            x = x0 + vx * tx;
        }
        if (x < XMIN) {
            tx = 0.0;
            x0 = XMIN;
            vx = -vx;
            x = x0 + vx * tx;
        }

        pthread_mutex_lock(&mxa);
        draw_ball(ox, YWIN - oy, BGC);
        draw_ball(x, YWIN - y, col);
        pthread_mutex_unlock(&mxa);

        oy = y;
        ox = x;
        t += dt;
        tx += dt;

        /* check for deadline miss */
        if (ptask_deadline_miss()) {
            dcol = dcol % 15 + 1;
            pthread_mutex_lock(&mxa);
            rectfill(screen, 400, 50, 450, 70, dcol);
            pthread_mutex_unlock(&mxa);
        }
        ptask_wait_for_period();
    }
}

/*--------------------------------------------------------------*/
/*			MAIN process				*/
/*--------------------------------------------------------------*/

int main(void) {
    int c;                              /* character from keyboard	*/
    int i, j, k;                        /* number of tasks created	*/
    double a;                           /* temporary variable           */
    int h;                              /* temporary variable           */
    int ntasks = 0;                     /* total number of created tasks*/
    int last_proc = 0;                  /* last assigned processor      */
    int max_proc = ptask_getnumcores(); /* max number of procs  */

    init();

    a = 2. * G * (float)TOP;
    for (i = 0; i < MAX_TASKS; i++)
        v0[i] = sqrt(a);

    i = 0;
    do {
        k = 0;
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }

        if ((ntasks == 0) && (k == KEY_SPACE)) {
            clear_to_color(screen, BGC);
            rect(screen, XMIN - L - 1, BASE - 1, XMAX + L + 1,
                 TOP + BASE + L + 1, 14);
        }

        if ((ntasks < MAX_TASKS) && (k == KEY_SPACE)) {
            tpars params = TASK_SPEC_DFL;
            params.period = tspec_from(PER, MILLI);
            params.rdline = tspec_from(DREL, MILLI);
            params.priority = PRIO - i;
            params.measure_flag = 1;
            params.act_flag = NOW;
            /* a round robin assignment */
            params.processor = last_proc++;
            if (last_proc >= max_proc)
                last_proc = 0;

            /** i = task_create(palla, PER, DREL, PRIO-i, NOW); */
            i = ptask_create_param(palla, &params);
            if (i != -1) {
                printf("Task %d created and activated\n", i);
                ntasks++;
            } else {
                allegro_exit();
                printf("Error in creating task!\n");
                exit(-1);
            }
        }

        if ((k >= KEY_0) && (k <= KEY_9)) {
            a = 2. * G * (float)TOP;
            pthread_mutex_lock(&mxv);
            v0[k - KEY_0] = sqrt(a);
            pthread_mutex_unlock(&mxv);
        }

        if ((k == KEY_O) && (ntasks > 9)) {
            for (j = 10; j < ntasks; j++) {
                h = rand() % (TOP - BASE);
                a = 2. * G * (float)h;
                pthread_mutex_lock(&mxv);
                v0[j] = sqrt(a);
                pthread_mutex_unlock(&mxv);
            }
        }

        if (k == KEY_A) {
            for (j = 0; j < ntasks; j++) {
                h = rand() % (TOP - BASE);
                a = 2. * G * (float)h;
                pthread_mutex_lock(&mxv);
                v0[j] = sqrt(a);
                pthread_mutex_unlock(&mxv);
            }
        }

        /*
          Printing deadline misses
          TO BE DONE
          for (j=0; j<ntasks; j++) {
          sprintf(s, "%d", task_dmiss(j));
          textout_ex(screen, font, s, 50+j*48, 450, 7, 0);
          }
        */

    } while (k != KEY_ESC);

    printf("Now printing the stats\n");
    for (j = 0; j < ntasks; j++) {
        tspec wcet = ptask_get_wcet(j);
        tspec acet = ptask_get_avg(j);

        printf("TASK %d: WCET = %ld\t ACET = %ld\t NINST=%d\n", j,
               tspec_to(&wcet, MICRO), tspec_to(&acet, MICRO),
               ptask_get_numinstances(j));
    }

    printf("End of statistics\n");
    allegro_exit();
    return 0;
}

/*--------------------------------------------------------------*/
