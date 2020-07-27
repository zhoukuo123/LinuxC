#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

sem_t sem[3];

void *func1(void *arg) {
    while (1) {
        sem_wait(&sem[0]);
        printf("A");
        sem_post(&sem[1]);
    }
}

void *func2(void *arg) {
    while (1) {
        sem_wait(&sem[1]);
        printf("B");
        sem_post(&sem[2]);
    }
}

void *func3(void *arg) {
    while (1) {
        sem_wait(&sem[2]);
        printf("C");
        sem_post(&sem[0]);
    }
}

int main() {
    pthread_t tid[3];

    sem_init(&sem[0], 0, 1);
    sem_init(&sem[1], 0, 0);
    sem_init(&sem[2], 0, 0);

    pthread_create(&tid[0], NULL, func1, NULL);
    pthread_create(&tid[1], NULL, func2, NULL);
    pthread_create(&tid[2], NULL, func3, NULL);

    for (int i = 0; i < 3; ++i) {
        pthread_join(tid[i], NULL);
    }

    for (int j = 0; j < 3; ++j) {
        sem_destroy(&sem[j]);
    }

    return 0;
}
