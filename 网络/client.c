#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>

#define SERV_PORT 9527

void sys_err(const char *string, int line) {
    fprintf(stderr, "line:%d ", line);
    perror(string);
    exit(1);
}

int main() {
    int cfd;
    int result;
    int count = 10;
    char buf[BUFSIZ];

    struct sockaddr_in serv_addr; // 服务器的地址结构
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        sys_err("socket", __LINE__);
    }

    result = connect(cfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (result == -1) {
        sys_err("connect", __LINE__);
    }

    while (count--) {
        write(cfd, "hello\n", 6);
        result = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, result);
        sleep(1);
    }

    close(cfd);

    return 0;
}