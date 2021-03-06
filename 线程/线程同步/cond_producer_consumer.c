#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define CAPACITY 10

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
int size;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    struct msg *mp = (struct msg *) malloc(sizeof(struct msg));
    int n = 10;
    while (n--) {
        mp->num = rand() % 1000 + 1;

        while (size >= CAPACITY) { // 缓冲区满了
            printf("size >= CAPACITY缓冲区满了\n");
            sleep(rand() % 3);
        }

        pthread_mutex_lock(&mutex);

        mp->next = head;
        head = mp;

        size++;

        pthread_mutex_unlock(&mutex);

        printf("--produce id =%lu produce %d size=%d\n", pthread_self(), mp->num, size);

        pthread_cond_signal(&has_data); // 将阻塞在该条件变量上的一个线程唤醒
        sleep(rand() % 3);
    }
    return NULL;
}

void *consumer(void *arg) {
    struct msg *mp;
    pthread_mutex_lock(&mutex); // 加锁
    int n = 10;
    while (n--) {
        if (head == NULL) {
            pthread_cond_wait(&has_data, &mutex); //  阻塞等待条件变量, 判断条件是否满足
        }

        mp = head;
        head = mp->next;
        size--;
        pthread_mutex_unlock(&mutex);

        printf("--consume %d size=%d\n", mp->num, size);
        free(mp);
        sleep(rand() % 3);
    }

    return NULL;
}

int main() {
    pthread_t pid, cid;
    int ret;
    srand(time(NULL));
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0) {
        err_thread(ret, "pthread_create producer error");
    }
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