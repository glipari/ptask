#define _GNU_SOURCE
#include "animation.h"
#include "testParameters.h"
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <sys/resource.h>
#include <sys/time.h>

void draw_ball(int x, int y, int c) { circlefill(screen, x, y, L, c); }

void print_id_task(int id, int x, int y, int color) {

    textprintf_ex(screen, font, x, y, color, BGC, "T%d", id);
}

void periodicBall_testParam() {

    int i, j = 0;
    int dcol = BGC;
    int *pun_col;
    int color;
    int x, ox, xStart;
    int y, oy;
    float vel = VELX;
    float tx = 0.0, dt;

    i = ptask_get_index();
    pun_col = ptask_get_argument();
    color = *pun_col;

    x = ox = xStart = XMIN;
    y = oy = BASE - 1 + 20 + (i * 4 * L);
    dt = ptask_get_period(i, MILLI) / 100.;

    /* print task's id on screen on the right of task animation */
    pthread_mutex_lock(&mxa);
    print_id_task(i, 5, y, color);
    pthread_mutex_unlock(&mxa);

    sample[i] = 0;
    while (1) {

        /* update position x of the ball */
        x = xStart + vel * tx;

        if (x > XMAX) {
            tx = 0.0;
            xStart = XMAX;
            vel = -vel;
            x = xStart;
        }
        if (x < XMIN) {
            tx = 0.0;
            xStart = XMIN;
            vel = -vel;
            x = xStart;
        }

        pthread_mutex_lock(&mxa);
        draw_ball(ox, y, BGC);
        draw_ball(x, y, color);
        pthread_mutex_unlock(&mxa);

        dt = ptask_get_period(i, MILLI) / 100.;
        tx += dt;
        ox = x;

        if (ptask_deadline_miss()) {
            /* draw with a different color a circle that represent deadline miss
             */
            dcol = (dcol + 1) % 15;
            pthread_mutex_lock(&mxa);
            draw_ball(XMAX + 15, y, dcol);
            pthread_mutex_unlock(&mxa);
        }

        ptask_wait_for_period();

        /* get sample of start time and next_activation time (not more than
         * MAX_SAMPLE)*/
        if (j < MAX_SAMPLE) {
            start_time[i][j] = ptask_gettime(MILLI);
            v_next_at[i][j] = ptask_get_nextactivation(MILLI);
            sample[i]++;
            j++;
        }
    }
}

void periodicBall_testParamDEP_PER() {

    int i, j = 0;
    int dcol = BGC;
    int *pun_col;
    int color;
    int x, ox, xStart;
    int y, oy;
    float vel = VELX;
    float tx = 0.0;
    float dt = 20.0 / 100.0; /* dt is constant */

    i = ptask_get_index();
    pun_col = ptask_get_argument();
    color = *pun_col;

    x = ox = xStart = XMIN;
    y = oy = BASE - 1 + 20 + (i * 4 * L);

    /* print task's id on screen on the right of task animation */
    pthread_mutex_lock(&mxa);
    print_id_task(i, 5, y, color);
    pthread_mutex_unlock(&mxa);

    sample[i] = 0;
    while (1) {

        /* update position x of the ball */
        x = xStart + vel * tx;

        if (x > XMAX) {
            tx = 0.0;
            xStart = XMAX;
            vel = -vel;
            x = xStart;
        }
        if (x < XMIN) {
            tx = 0.0;
            xStart = XMIN;
            vel = -vel;
            x = xStart;
        }

        pthread_mutex_lock(&mxa);
        draw_ball(ox, y, BGC);
        draw_ball(x, y, color);
        pthread_mutex_unlock(&mxa);

        tx += dt;
        ox = x;

        if (ptask_deadline_miss()) {
            /* draw with a different color a circle that represent deadline miss
             */
            dcol = (dcol + 1) % 15;
            pthread_mutex_lock(&mxa);
            draw_ball(XMAX + 15, y, dcol);
            pthread_mutex_unlock(&mxa);
        }

        ptask_wait_for_period();

        /* get sample of start time and next_activation time (not more than
         * MAX_SAMPLE)*/
        if (j < MAX_SAMPLE) {
            start_time[i][j] = ptask_gettime(MILLI);
            v_next_at[i][j] = ptask_get_nextactivation(MILLI);
            sample[i]++;
            j++;
        }
    }
}
