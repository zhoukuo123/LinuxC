#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;

void *thread_func(void *arg) {
    int flag = (int) arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        if (flag == 1) {
            printf("%c", 'A');
            flag = 2;
        } else if (flag == 2) {
            printf("%c", 'B');
            flag = 3;
        } else if (flag == 3) {
            printf("%c", 'C');
            flag = 1;
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t tidA, tidB, tidC;
    int flag = 1;
    int ret = pthread_mutex_init(&mutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "mutex init error:%s\n", strerror(ret));
        exit(1);
    }
    pthread_create(&tidA, NULL, thread_func, (void *) flag);
    pthread_create(&tidB, NULL, thread_func, (void *) flag);
    pthread_create(&tidC, NULL, thread_func, (void *) flag);

    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
    pthread_join(tidC, NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}