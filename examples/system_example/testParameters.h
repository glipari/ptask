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

#define L 4 /* dimension of a ball or hight line */

#define NUM_T_TEST 7
#define MAX_TASKS 50

#define INIZIALIZED 0
#define DIFFERENT 1

#define NUM_CIF_PRIO 2
#define W_EDIT_PRIO 2 + NUM_CIF_PRIO // spazio + NUM_CIF_PRIO + 2 spazi
#define W_TEXT_PRIO 3                // "PXX: "
#define PIXEL_CHAR 8                 // circa
#define ON 1
#define OFF 0

#define PRIO_EQUAL 0
#define PRIO_DIFF 1
#define PRIO_CUSTOM 2

#define SCHED 0
#define PART 1
#define PROT 2

#define BASE 60                         /* position of the floor		*/
#define TOP BASE + 15 * NUM_T_TEST + 20 /* initial height of the ball	*/
#define BASE1 TOP + 25
#define TOP1 BASE1 + 15 * NUM_T_TEST + 40
#define XMIN 40  /* min position X of the ball	*/
#define XMAX 600 /* max position Y of the ball	*/

#define PER 20 /* task period in ms			*/

#define PRIO 80 /* task priority				*/

#define VELX 15. /* horizontal ball velocity		*/

/* DRAW GRID AND TASK */
#define XMIN_GRID 0
#define YMIN_GRID 0
#define XMAX_GRID 640
#define YMAX_GRID 480
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
/* ************************************************************* */
/* mutual exclusion semaphores  */
pthread_mutex_t mxa; // = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mx_sezNorm; // no protocol
pthread_mutex_t mx_sezA;
pthread_mutex_t mx_sezB;
