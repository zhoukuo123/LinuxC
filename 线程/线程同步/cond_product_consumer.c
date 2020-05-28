#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void err_thread(int ret, char *str) {
    if (ret != 0) {
        fprintf(stderr, "%s:%s\n", str, strerror(ret));
        pthread_exit(NULL);
    }
}

struct msg {
    int num;
    struct msg *next;
};

struct msg *head;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    struct msg *mp;
    return NULL;
}

void *consumer(void *arg) {
    pthread_mutex_lock(&mutex); // 加锁
    if (head == NULL) {
        pthread_cond_wait(&has_data, &mutex); //  阻塞等待条件变量, 判断条件是否满足
    }

    return NULL;
}

int main() {
    pthread_t pid, cid;
    int ret;
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
    ret = pthread_create(&cid, NULL, consumer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create consumer error");
    }

    pthread_join(pid, NULL);
    pthread_join(cid, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&has_data);

    return 0;
}