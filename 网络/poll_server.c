#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 8000
#define OPEN_MAX 1024

int main() {
    int maxi, listenfd, connfd, sockfd, i, j;
    int nready;
    ssize_t n;

    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t clilen;

    struct pollfd client[OPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    Listen(listenfd, 128);

    client[0].fd = listenfd;
    client[0].events = POLLIN;

    for (i = 1; i < OPEN_MAX; ++i) {
        client[i].fd = -1;
    }

    maxi = 0;

    while (1) {
        nready = poll(client, maxi + 1, -1); // 阻塞监听是否有客户端连接请求
        if (nready < 0) {
            perr_exit("poll");
        }

        if (client[0].revents & POLLIN) { // listenfd有读事件就绪
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen); // 接受客户端连接请求, Accept不会阻塞

            for (i = 1; i < OPEN_MAX; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }

            if (i == OPEN_MAX) {
                perr_exit("too many clients");
            }

            client[i].events = POLLIN;
            if (i > maxi) { // 更新client[]最大元素下标
                maxi = i;
            }
            if (nready == 1) { // 没有更多就绪事件时, 继续回到poll阻塞
                continue;
            }
        }

        for (i = 1; i <= maxi; ++i) { // 检测client[]是那个connfd有事件就绪
            sockfd = client[i].fd;
            if (client[i].revents & POLLIN) {
                if ((n = Read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) { // 连接被重置
                        printf("client[%d] aborted connection\n", i);
                        Close(sockfd);
                        client[i].fd = -1;
                    } else {
                        perr_exit("read");
                    }
                } else if (n == 0) { // 客户端关闭连接
                    printf("client[%d] closed connection\n", i);
                    Close(sockfd);
                    client[i].fd = -1;
                } else {
                    for (j = 0; j < n; j++) {
                        buf[j] = toupper(buf[j]);
                    }
                    Write(sockfd, buf, n);
                }
                if (nready == 1) {
                    break;
                }
            }
        }
    }
    return 0;
}