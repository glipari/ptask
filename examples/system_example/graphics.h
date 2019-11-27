#include "testParameters.h"
#include <stdbool.h>
#include <string.h>

typedef struct {
    int sched;
    int part;
    int prot;
} parametri;

parametri start_dialog();
void draw_system_info(int s, int part, int prot, int p, int tipo_test);
void draw_activation(int numTActive, int idT, int prio, bool verbose);
void init_vettore_prio(int tipo_prio, int *prio, int dim);
int select_sched();
int select_part();
int select_prot();
