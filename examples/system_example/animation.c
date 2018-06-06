#define _GNU_SOURCE
#include "animation.h"
#include "testParameters.h"
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <sys/resource.h>
#include <sys/time.h>

void draw_ball(int x, int y, int c) { circlefill(screen, x, y, L, c); }

/*--------------------------------------------------------------*/
/*	Periodic task			*/
/*--------------------------------------------------------------*/

void periodicBall_testPart() {
    int i, col; /* id task  */
    int x, y;   /* x, y ball coordinates */
    int ox;     /* old x coordinate   */
    int x0;     /* start x position ball  */
    float vx;   /* ball speed   */
    float tx;   /* time variable    */
    float dt;   /* time increment    */

    int offset;

    i = ptask_get_index();
    col = 2 + i % 14;
    offset = 20;
    y = BASE + offset + (L * i * 3);
    x = ox = x0 = XMIN;

    vx = VELX;
    tx = 0.0;
    dt = ptask_get_period(i, MILLI) / 100.;

    pthread_mutex_lock(&mxa);
    textprintf_ex(screen, font, 5, y, FGC, BGC, " T%d", i + 1);
    textprintf_ex(screen, font, XMIN + 10, BASE1 + 30 + (15 * i), FGC, BGC,
                  " T%d run on core - %d -", i + 1, sched_getcpu());
    pthread_mutex_unlock(&mxa);

    while (1) {

        x = x0 + vx * tx;

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
        draw_ball(ox, y, BGC);
        draw_ball(x, y, col);
        textprintf_ex(screen, font, XMIN + 10 + 144, BASE1 + 30 + (15 * i), FGC,
                      BGC, "%d", sched_getcpu());
        pthread_mutex_unlock(&mxa);

        tx += dt;
        ox = x;

        ptask_wait_for_period();
    }
}
