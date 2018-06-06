#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static __thread int myindex;

void *body(void *arg) {
    myindex = *((int *)arg);

    printf("This is my index : %d\n", myindex);
    myindex += myindex;

    printf("This is my index : %d\n", myindex);
    return 0;
}

int main() {
    int i;
    pthread_t tid[4];
    pthread_t args[4];

    for (i = 0; i < 4; ++i) {
        args[i] = i + 1;
        pthread_create(&tid[i], 0, body, &args[i]);
    }

    printf("All threads have been created\n");
    for (i = 0; i < 4; ++i)
        pthread_join(tid[i], 0);

    printf("Exit\n");
    return 0;
}
