#define _GNU_SOURCE
#include "animation.h"
#include "graphics.h"
#include "testParameters.h"
#include <stdbool.h>
#include <string.h>

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
    bool state_task[NUM_T_TEST]; /* state_i==true task created*/
    int priority[NUM_T_TEST];
    int i, id, k;
    int c;          /* character from keyboard	*/
    int ntasks = 0; /* total number of activated tasks*/
    int ret = 0;
    int tipo_prio = PRIO_DIFF;
    int sched = SCHED_OTHER;
    int prot = NO_PROTOCOL;
    int test = PART;

    for (i = 0; i < NUM_T_TEST; i++) {
        state_task[i] = false;
    }

    init();

    ret = select_part();
    if (ret == -1) {
        allegro_exit();
        return 0;
    }

    init_vettore_prio(tipo_prio, priority, NUM_T_TEST);

    ptask_init(sched, ret, prot);

    draw_system_info(sched, ret, prot, tipo_prio, test);

    /*Creation aperiodic task*/
    for (i = 0; i < NUM_T_TEST; i++) {
        tpars params = TASK_SPEC_DFL;
        params.period = tspec_from(PER, MILLI);
        params.priority = priority[i];
        params.measure_flag = 0;
        params.act_flag = DEFERRED;
        params.processor = 0;
        id = ptask_create_param(periodicBall_testPart, &params);
        if (id != -1) {
            printf("Task %d created \n", id);
            state_task[id] = true; // task create state=true
        } else {
            allegro_exit();
            printf("Error in creating task!\n");
            exit(-1);
        }
    }

    do {
        k = 0;
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }

        /*section for task activation*/
        if ((k >= KEY_1) && (k <= KEY_0 + NUM_T_TEST)) {

            id = k - KEY_0 - 1;
            if (state_task[id]) { // if task is create but not activate
                ret = ptask_activate(id);
                if (ret != -1) {
                    printf("Task %d activated\n", id);
                    draw_activation(ntasks, id, priority[id], false);
                    ntasks++;
                    state_task[id] = false;
                }
            }
        }

    } while (k != KEY_ESC);

    pmux_destroy(&mxa);
    allegro_exit();
    return 0;
}

/*--------------------------------------------------------------*/
