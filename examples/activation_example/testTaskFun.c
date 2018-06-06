#include <stdbool.h>
#include <string.h>
#include "animation.h"
#include "graphics.h"
#include "testParameters.h"

void init() {
	allegro_init();

	/* Initializes this library */
	tspec_init();

	set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
	clear_to_color(screen, BGC);
	install_keyboard();
	install_mouse();
	srand(time(NULL));

	/* semaphore to write on the screen */
	pmux_create_pi(&mxa);
}

/*--------------------------------------------------------------*/
/*			MAIN process										*/
/*--------------------------------------------------------------*/

int	main(void)
{
    int 	unit = MILLI;
    int	i, id, k;
    int	c;						/* character from keyboard	*/
    int   ntasks 	= 0;        	/* total number of activated tasks*/
    int 	ret		= 0;
    int 	tipo_prio = PRIO_EQUAL;
    int 	sched 	= SCHED_RR;
    int 	part	= GLOBAL;
    int 	prot 	= NO_PROTOCOL;
    int 	test 	= TASK_FUN;
    int 	modeACT = MOD_NOW;		/* {MOD_NOW=0, MOD_DEF_OFFSET=1, MOD_DEF_NO_OFFS = 2} */
    int   act_flag = NOW;

    init();

    int 	num_tasks = NUM_T_TEST;
    int 	priority[num_tasks];
    ptime v_offset[num_tasks];	/* time offset vector */

    for(i = 0; i < num_tasks; i++) v_offset[i] = 0;

    modeACT = select_act (v_offset, num_tasks); // {MOD_NOW=0, MOD_DEF_OFFSET=1, MOD_DEF_NO_OFFS = 2}
    if ( modeACT == -1 ) {
        allegro_exit();
        return 0;
    }

    ptask_init(sched, part, prot);

    /* Time reference for timeoffset */
    time_t0 = ptask_gettime(MILLI);
    ptime time_tmp;

    if(modeACT == 0) {	// {MOD_NOW=0, MOD_DEF_OFFSET=1, MOD_DEF_NO_OFFS = 2}
        act_flag = NOW;
    }
    else {
        act_flag = DEFERRED;
    }

    init_vettore_prio(tipo_prio, priority, num_tasks);

    draw_system_info(sched, part, prot, modeACT, tipo_prio, test, false);

    draw_Timetask_info(modeACT);

    pthread_mutex_lock(&mxa);
    textprintf_ex(screen, font, 2, 25 + 15, FGC, BGC, " Time reference t0 = %ld (time creation/visualization of this screen)", time_t0);
    pthread_mutex_unlock(&mxa);
    const char* str_offset = "Time offset(ms) inseriti = ";

    /* Creation of aperiodic task */
    for ( i = 0; i < num_tasks; i++) {

        if (modeACT != MOD_DEF_OFFSET) {

            /* MODE WITHOUT OFFSET */
            time_tmp = ptask_gettime(MILLI);
            if(act_flag == NOW) {
                pthread_mutex_lock(&mxa);
                textprintf_ex(screen, font, X_LINE_VERT0 + 5, Y_TIMES + i * ALT_RIGA, FGC, BGC, "%ld            ", time_tmp);
                pthread_mutex_unlock(&mxa);
            }
            id = ptask_create_prio(periodicLine_testSystemTask, PER, priority[i], act_flag);
        }
        else {
            /* MODE WITH OFFSET */
            time_tmp = ptask_gettime(MILLI);
            pthread_mutex_lock(&mxa);
            textprintf_ex(screen, font, (X_LINE_VERT0 + X_LINE_VERT1)/2, Y_TIMES + i * ALT_RIGA, FGC, BGC, "-");
            pthread_mutex_unlock(&mxa);
            id = ptask_create_prio(periodicLine_testSystemTaskOFFSET, PER, priority[i], act_flag);
            printf("act_flag = %d, id = %d, v_offset[%d] = %ld\n", act_flag, id+1, id, v_offset[id]);
            ret = ptask_activate_at(id, v_offset[i], MILLI);

            if (ret != -1) {
                draw_activation (ntasks, i, priority[i], false);
                ntasks++;
                if (i == 0) {
                    pthread_mutex_lock(&mxa);
                    textout_ex(screen, font, str_offset, XMIN, BASE1 + 2 * ALT_RIGA, FGC, 0);
                    pthread_mutex_unlock(&mxa);
                }
                pthread_mutex_lock(&mxa);
                textprintf_ex(screen, font, XMIN + (strlen(str_offset)+i*(NUM_CIF_OFFSET))*PIXEL_CHAR, BASE1 + 2 * ALT_RIGA,FGC, BGC, "%ld", v_offset[i]);
                pthread_mutex_unlock(&mxa);
            }
            else fprintf(stderr, "Task %d non può essere attivato o già avviato!!!\n", i+1);

        }
        if (id != -1) {
            char* temp = " e attivato!\n";
            if(act_flag == DEFERRED) {
                temp = "!\n";
            }
            printf("Task %d creato%s", id+1, temp);
        }
        else {
            allegro_exit();
            printf("Errore nella creazione(o anche attivazione NOW) del task!\n");
            exit(-1);
        }
    }

    do {

        k = 0;
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }

        if ((k >= KEY_1) && (k <= KEY_0 + num_tasks) && (modeACT != MOD_DEF_OFFSET)) {

            id = k - KEY_0 - 1;
            time_tmp = ptask_gettime(unit);
            ret = ptask_activate(id);

            if (ret != -1) {
                printf("Task %d attivato\n", id);
                pthread_mutex_lock(&mxa);
                textprintf_ex(screen, font, X_LINE_VERT0 + 5, Y_TIMES + (id) * ALT_RIGA,
                              FGC, BGC, "%ld            ", time_tmp);
                pthread_mutex_unlock(&mxa);
                draw_activation (ntasks, id, priority[id], false);
                ntasks++;
            }
            else{

                printf("Task %d non può essere attivato o già avviato!!!\n", id);
                pthread_mutex_lock(&mxa);
                textprintf_ex(screen, font, X_LINE_VERT0 + 5, Y_TIMES + id * ALT_RIGA,FGC, BGC, "T%d: Gia' attivo!   ", id);
                pthread_mutex_unlock(&mxa);
            }
        }

    } while (k != KEY_ESC);
    allegro_exit();
    return 0;
}
END_OF_MAIN()
/*--------------------------------------------------------------*/

