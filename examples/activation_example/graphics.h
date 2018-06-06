#include "testParameters.h"
#include <stdbool.h>
#include <string.h>

void draw_system_info(int s, int part, int prot, int act_mode, int p,
                      int tipo_test, bool verbose);

/* draw the identifier of the task activated */
void draw_activation(int numTActive, int idT, int prio, bool verbose);

/* Draw time information about task activations and their start time and offset
 */
/* The time offset are time intervals from 'ptask_init(...)' to activation of
 * tasks*/
// modeACT={MOD_NOW=0, MOD_DEF_OFFSET=1, MOD_DEF_NO_OFFS = 2}
void draw_Timetask_info(int modeACT);

void init_vett(int tipo, int *vett, int dim, int val);

/*type= {PRIO_EQUAL = 0: task with equal priority,
 * 		 PRIO_DIFF  = 1: task with different priority,
 * 		 PRIO_CUSTOM= 2: task with customizable priority} */
void init_vettore_prio(int tipo, int *prio, int dim);

/* Errors checking on edit fields */
int control_bound_value(int val, bool *error_found, const char *name_value,
                        int upper_bound);

/* Errors checking on edit fields */
bool error_edit(char *string, const char *name_value, char *endptr, int val,
                int upper_bound);

/* Function to select type of activation mode */
int select_act(ptime *v_offset, int numTask);
