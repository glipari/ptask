#include "testParameters.h"

#define MAX_SAMPLE 10

extern ptime start_time[NUM_T_TEST][MAX_SAMPLE];
extern ptime v_next_at[NUM_T_TEST][MAX_SAMPLE];
extern int sample[NUM_T_TEST]; // sample[i]: number of sample processed of the taski

void periodicBall_testParam();
void periodicBall_testParamDEP_PER(); // movement of task is "dependent" from
                                      // his period
