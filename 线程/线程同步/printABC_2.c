// 条件变量+互斥锁
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_cond_t printed[3];
pthread_mutex_t mutex[3];

void *func1(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex[0]);
        pthread_cond_wait(&printed[0], &mutex[0]);
        putchar('A');
        pthread_mutex_unlock(&mutex[0]);
        pthread_cond_signal(&printed[1]);
    }
}

void *func2(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex[1]);
        pthread_cond_wait(&printed[1], &mutex[1]);
        putchar('B');
        pthread_mutex_unlock(&mutex[1]);
        pthread_cond_signal(&printed[2]);
    }
}

void *func3(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex[2]);
        pthread_cond_wait(&printed[2], &mutex[2]);
        putchar('C');
        pthread_mutex_unlock(&mutex[2]);
        pthread_cond_signal(&printed[0]);
    }
}

int main() {
    pthread_t tid[3];

    for (int i = 0; i < 3; ++i) {
        pthread_cond_init(&printed[i], NULL);
        pthread_mutex_init(&mutex[i], NULL);
    }

    pthread_create(&tid[0], NULL, func1, NULL);
    pthread_create(&tid[1], NULL, func2, NULL);
    pthread_create(&tid[2], NULL, func3, NULL);

    pthread_cond_signal(&printed[0]);

    for (int j = 0; j < 3; ++j) {
        pthread_join(tid[j], NULL);
    }

    for (int k = 0; k < 3; ++k) {
        pthread_mutex_destroy(&mutex[k]);
        pthread_cond_destroy(&printed[k]);
    }

    return 0;
}
