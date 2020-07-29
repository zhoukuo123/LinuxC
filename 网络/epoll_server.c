#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include "wrap.h"

#define MAXLINE 8192
#define SERV_PORT 8000
#define OPEN_MAX 5000

int main(int argc, char *argv[]) {
    int i, listenfd, connfd, sockfd;
    int n, num = 0;
    ssize_t nready, efd, res;
    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t clilen;

    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 端口复用
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    Listen(listenfd, 20);

    efd = epoll_create(OPEN_MAX);
    if (efd == -1) {
        perr_exit("epoll_create error");
    }

    struct epoll_event tep, ep[OPEN_MAX];

    tep.events = EPOLLIN;
    tep.data.fd = listenfd;
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
    if (res == -1) {
        perr_exit("epoll_ctl error");
    }

    while (1) {
        nready = epoll_wait(efd, ep, OPEN_MAX, -1); // 阻塞监听
        if (nready == -1) {
            perr_exit("epoll_wait error");
        }

        for (i = 0; i < nready; ++i) {
            if (ep[i].data.fd == listenfd) {
                clilen = sizeof(cliaddr);
                connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

                tep.events = EPOLLIN;
                tep.data.fd = connfd;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
                if (res == -1) {
                    perr_exit("epoll_ctl error");
                }
            } else { // 不是lfd
                sockfd = ep[i].data.fd;
                n = Read(sockfd, buf, MAXLINE);

                if (n == 0) { // 读到0, 说明客户端关闭连接, 将该文件描述符从红黑树中移除
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    if (res == -1) {
                        perr_exit("epoll_ctl error");
                    }
                    Close(sockfd); // 关闭与该客户端的连接
                    printf("client[%d] closed connection\n", sockfd);
                } else if (n < 0) { // 出错
                    perror("read n < 0 error: ");
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    Close(sockfd);
                } else {
                    for (i = 0; i < n; ++i) {
                        buf[i] = toupper(buf[i]);
                    }

                    Write(STDOUT_FILENO, buf, n);
                    Write(sockfd, buf, n);
                }
            }
        }
    }
    Close(listenfd);
    Close(efd);
    return 0;
}



