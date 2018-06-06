#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pmutex.h"
#include "ptask.h"
#include "tstat.h"

#define XWIN 640
#define YWIN 480

#define BGC 0  /* background color	*/
#define FGC 15 /* foreground color */

#define L 5 /* dimension of a ball or hight line */

#define NUM_T_TEST 3

#define INIZIALIZED 0
#define DIFFERENT 1

#define NUM_CIF_OFFSET 6 // offset (usually in ms)
#define W_EDIT_OFFSET 2 + NUM_CIF_OFFSET
#define W_TEXT_OFFSET 6 // "OffsXX: "

#define PIXEL_CHAR 8
#define ON 1
#define OFF 0
#define OFFSET_MAX 60000

#define PRIO_EQUAL 0 /* Modes of priority */
#define PRIO_DIFF 1
#define PRIO_CUSTOM 2

#define OFFSET_EQUAL 0 /* Modes of OFFSET */
#define OFFSET_DIFF 1
#define OFFSET_CUSTOM 2

#define MOD_NOW 0
#define MOD_DEF_OFFSET 1
#define MOD_DEF_NO_OFFS 2

#define SCHED 0 /* Types of test */
#define PART 1
#define PROT 2
#define TASK_FUN 3

#define BASE 60                         /* position of the floor		*/
#define TOP BASE + 15 * NUM_T_TEST + 20 /* initial height of the ball	*/
#define BASE1 TOP + 25
#define TOP1 BASE1 + 15 * NUM_T_TEST + 40
#define XMIN 40  /* min position X of the ball	*/
#define XMAX 600 /* max position Y of the ball	*/

/* Parameters for draw times of the functions task*/
#define YMAX1 TOP1 + L + 10 + 1
#define ALT_RIGA 10                /* height row in pixels */
#define ALT_MEZZARIGA ALT_RIGA / 2 /* height row/2 in pixels */
#define X_LINE_VERT0 XMIN + 25     /* coordinats x of first vertical line */
#define X_LINE_VERT1                                                           \
    (((0.0f + XMAX) - (0.0f + XMIN)) / (3)) +                                  \
        XMIN /* coordinats x of second vertical line */
#define X_LINE_VERT2                                                           \
    (X_LINE_VERT1 - XMIN) * 2 + XMIN /* coordinats x of third vertical line */
#define Y_ORRIZ_LINE BASE1 + 5 * ALT_RIGA /* height of horrizontal line */
#define Y_TIMES Y_ORRIZ_LINE + ALT_MEZZARIGA

#define PER 100  /* task period in ms			*/
#define DREL 100 /* realtive deadline in ms		*/
#define PRIO 80  /* task priority				*/

ptime time_t0; /* Time reference for timeoffset (from ptask_init(...))*/

/* *************************************************************************************************
 */

/* mutual exclusion semaphores  */
pthread_mutex_t mxa; // = PTHREAD_MUTEX_INITIALIZER;
