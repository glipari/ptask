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

#define NUM_T_TEST 9 /* 1 <= NUM_T_TEST <= 9 */

#define PIXEL_CHAR 8

#define PRIO_EQUAL 0
#define PRIO_DIFF 1
#define PRIO_CUSTOM 2

#define SCHED 0
#define PART 1
#define PROT 2

#define PARAM_PERIOD 0
#define PARAM_DEADLINE 1
#define PARAM_CPU 2
#define PARAM_PRIORITY 3

#define BASE 60                         /* position of the floor		*/
#define TOP BASE + 15 * NUM_T_TEST + 20 /* initial height of the ball	*/
#define BASE1 TOP + 25
#define TOP1 BASE1 + 15 * NUM_T_TEST + 40
#define XMIN 40  /* min position X of the ball	*/
#define XMAX 600 /* max position Y of the ball	*/

#define PER 20  /* task period in ms			*/
#define DREL 20 /* relative deadline in ms		*/
#define PRIO 80 /* task priority				*/

#define VELX 15. /* horizontal ball velocity		*/

/* mutual exclusion semaphores  */
pthread_mutex_t mxa; // = PTHREAD_MUTEX_INITIALIZER;

char index_modification[3];  /* contains index of task from which change
                                parameters */
char value_modification[10]; /* contains value of parameter to modify */
