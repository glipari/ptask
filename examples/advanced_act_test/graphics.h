#include "testParameters.h"
#include <stdbool.h>
#include <string.h>

void draw_system_info(int s, int part, int prot, int p, int tipo_test,
                      bool verbose);

/* draw the identifier of the task activated */
void draw_activation(int numTActive, int idT, int prio, bool verbose);

/*type= {PRIO_EQUAL = 0: task with equal priority,
 * 		 PRIO_DIFF  = 1: task with different priority,
 * 		 PRIO_CUSTOM= 2: task with customizable priority} */
void init_vettore_prio(int tipo, int *prio, int dim);

/* Creates and show dialog to modify dinamically parameters of tasks */
int modification_param();
