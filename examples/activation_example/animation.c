#define _GNU_SOURCE
#include "testParameters.h"
#include "animation.h"
#include <stdbool.h>
#include <sched.h>
#include <pthread.h>

/*--------------------------------------------------------------*/
/*	Periodic task 												*/
/*--------------------------------------------------------------*/

void periodicLine_testSystemTask() {

	int	ox;					/* old position  */
	int col;				/* line color */
	int offset = 0; 		/* offset from base0 and base 1*/
	int unit = MILLI;		/* unit of measurement for the time variables */
	int	i;					/* index of task  */
	int	x, y;				/* coordinates of the ball */
	char id[3];				/* string of index thread */
	bool bloccato = false;

	i = ptask_get_index();
	ptime time_activation 	= ptask_gettime(unit);

	col = 2 + i % 14;
	offset = 20;
	y= BASE + offset + (L * i * 3);
	x = ox = XMIN;

	snprintf(id, 3, "T%d", i+1);
	pthread_mutex_lock(&mxa);
	textprintf_ex(screen, font, X_LINE_VERT1 + 5, Y_TIMES + i * ALT_RIGA, FGC, BGC, "%ld", time_activation);
	textout_ex(screen, font, id, 5,y, FGC, 0);
	pthread_mutex_unlock(&mxa);

	while (x < XMAX) {

			x = x + 10;

			pthread_mutex_lock(&mxa);
			rectfill(screen, ox, y+L, x, y, col);
			pthread_mutex_unlock(&mxa);

			ox = x;
			ptask_wait_for_period();
			if((x >= XMAX/2) && !bloccato) {
				bloccato = true;
				fprintf(stderr, "Sono il thread%d, mi sono bloccato!!!\n", i);
				ptask_wait_for_activation();
				time_activation = ptask_gettime(unit);
				pthread_mutex_lock(&mxa);
				textprintf_ex(screen, font, X_LINE_VERT1 + 5, Y_TIMES + i * ALT_RIGA, FGC, BGC, "%ld", time_activation);
				pthread_mutex_unlock(&mxa);
			}
	}
}

void periodicLine_testSystemTaskOFFSET() {
	int	i;					/* index of task  */
	int	x, y;				/* coordinates of the ball  */
	int	ox;					/* old position  */
	int col;				/* line color */
	int offset = 0; 		/* offset from base0 and base 1 */
	int unit = MILLI;		/* unit of measurement for the time variables */
	char id[3];				/* string of index thread */

	ptime time_start = ptask_gettime(unit);

	ptime offset_misurato;

	i = ptask_get_index();

	offset_misurato = time_start - time_t0;

	fprintf(stderr, "---Thread%d: inizio l'esecuzione al time=%ldms!!!\n", i, time_start);

	col = 2 + i % 14;
	offset = 20;
	y= BASE + offset + (L * i * 3);
	x = ox = XMIN;

	snprintf(id, 3, "T%d", i+1);
	pthread_mutex_lock(&mxa);
	textprintf_ex(screen, font, X_LINE_VERT2 + 5, Y_TIMES + i * ALT_RIGA, FGC, BGC, "%ld", offset_misurato);
	textprintf_ex(screen, font, X_LINE_VERT1 + 5, Y_TIMES + i * ALT_RIGA, FGC, BGC, "%ld", time_start);
	textout_ex(screen, font, id, 5,y, FGC, 0);
	pthread_mutex_unlock(&mxa);

	while (x < XMAX) {

			x = x + 10;

			pthread_mutex_lock(&mxa);
			rectfill(screen, ox, y+L, x, y, col);
			pthread_mutex_unlock(&mxa);

			ox = x;
			ptask_wait_for_period();

	}
}
