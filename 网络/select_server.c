#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <ctype.h>
#include "wrap.h"

#define SERV_PORT 6666

int main() {
    int nready, n;
    int maxfd = 0;
    int listenfd, connfd;
    char buf[BUFSIZ];
    struct sockaddr_in clie_addr, serv_addr;
    socklen_t clie_addr_len;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    Listen(listenfd, 128);

    fd_set rset, allset; // rset 读事件文件描述符集合, allset 用来暂存

    maxfd = listenfd;

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); // 构造select监控文件描述符集

    while (1) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            perr_exit("select");
        }
        if (FD_ISSET(listenfd, &rset)) { // 说明有新的客户端连接请求
            clie_addr_len = sizeof(clie_addr);
            connfd = Accept(listenfd, (struct sockaddr *) &clie_addr, clie_addr_len); // Accept不会阻塞

            FD_SET(connfd, &allset);

            if (maxfd < connfd) {
                maxfd = connfd;
            }
            if (0 == --nready) { // 只有连接请求, 没有客户端写数据(只有listenfd有事件, 后续的for循环不需要执行
                continue;
            }
        }
        for (int i = listenfd + 1; i <= maxfd; ++i) { // 检测那个client有数据就绪
            if (FD_ISSET(i, &rset)) {
                if ((n = Read(i, buf, sizeof(buf))) == 0) {
                    Close(i);
                    FD_CLR(i, &allset); // 解除select对此文件描述符的监控
                } else if (n > 0) {
                    for (int j = 0; j < n; ++j) {
                        buf[j] = toupper(buf[j]);
                    }
                    Write(i, buf, n);
                }
            }
        }
    }
}
