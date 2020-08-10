#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAXLINE 10

int main() {
    int efd, i;
    int pfd[2];
    pid_t pid;
    char buf[MAXLINE], ch = 'a';

    pipe(pfd);
    pid = fork();

    if (pid == 0) { // 子进程, 写
        close(pfd[0]); // 关闭读端
        while (1) {
            //aaaa\n
            for (i = 0; i < MAXLINE / 2; ++i) {
                buf[i] = ch;
            }
            buf[i - 1] = '\n';
            ch++;
            //bbbb\n
            for (; i < MAXLINE; ++i) {
                buf[i] = ch;
            }
            buf[i - 1] = '\n';
            ch++;
            //aaaa\nbbbb\n
            write(pfd[1], buf, sizeof(buf));
            sleep(5);
        }
//        close(pfd[1]);
    } else if (pid > 0) { // 父进程, 读
        struct epoll_event event;
        struct epoll_event resevent[10];
        int res, len;

        close(pfd[1]);
        efd = epoll_create(10);

        event.events = EPOLLIN | EPOLLET; // ET 边缘触发: 缓冲区剩余未读尽的数据不会导致epoll_wait返回, 新的事件满足才会触发.
//        event.events = EPOLLIN; // LT 水平触发(默认): 缓冲区剩余未读尽的数据会导致epoll_wait返回.
        event.data.fd = pfd[0];
        epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &event);

        while (1) {
            res = epoll_wait(efd, resevent, 10, -1);
            if (resevent[0].data.fd == pfd[0]) {
                len = read(pfd[0], buf, MAXLINE / 2);
                write(STDOUT_FILENO, buf, len);
            }
        }
//        close(pfd[0]);
//        close(efd);
    } else {
        perror("fork");
        exit(-1);
    }

//    return 0;
}
