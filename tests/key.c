#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct mystruct {
    char array[5];
    int x;
    int y;
};

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

static void make_key() { pthread_key_create(&key, NULL); }

void allocate() {
    void *ptr;

    pthread_once(&key_once, make_key);
    if ((ptr = pthread_getspecific(key)) == NULL) {
        ptr = malloc(sizeof(struct mystruct));
        pthread_setspecific(key, ptr);
    }
}

void *body(void *arg) {
    allocate();
    struct mystruct *ptr = (struct mystruct *)pthread_getspecific(key);
    memcpy(ptr, arg, sizeof(struct mystruct));

    printf("TASK[%d]: My own data (%d, %d)\n", ptr->x, ptr->x, ptr->y);
    usleep(ptr->x);
    ptr->y = ptr->x + 100;
    printf("TASK[%d]: Completing (%d, %d)\n", ptr->x, ptr->x, ptr->y);

    return 0;
}

int main() {
    int i;
    struct mystruct obj[4] = {
        {"abcd", 0, 0}, {"abcd", 1, 0}, {"abcd", 2, 0}, {"abcd", 3, 0}};

    pthread_t tid[4];

    for (i = 0; i < 4; ++i)
        pthread_create(&tid[i], 0, body, &obj[i]);

    printf("All threads have been created\n");
    for (i = 0; i < 4; ++i)
        pthread_join(tid[i], 0);

    printf("Exit\n");
    return 0;
}
