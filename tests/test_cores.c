#include <ptask.h>
#include <stdio.h>

int main() {
    ptask_init(SCHED_DEADLINE, PARTITIONED, PRIO_INHERITANCE);
    printf("Number of system cores: %d\n", ptask_getnumcores());
    return 0;
}
