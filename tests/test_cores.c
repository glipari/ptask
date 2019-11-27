#include <ptask.h>
#include <stdio.h>

int main() {
    printf("Number of system cores: %d\n", ptask_getnumcores());
    return 0;
}
