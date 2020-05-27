#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;

void *thread_func(void *arg) {
    srand(time(NULL));

    while (1) {
        pthread_mutex_lock(&mutex);
        printf("hello ");
        sleep(rand() % 3);
        printf("world\n");
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 3);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    srand(time(NULL));
    int ret = pthread_mutex_init(&mutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "mutex init error:%s\n", strerror(ret));
        exit(1);
    }
    pthread_create(&tid, NULL, thread_func, NULL);
    while (1) {
        pthread_mutex_lock(&mutex);
        printf("HELLO ");
        sleep(rand() % 3);
        printf("WORLD\n");
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 3);

    }
    pthread_join(tid, NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}
