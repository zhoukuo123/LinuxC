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

#define  SERV_PORT 9527

void sys_err(const char *str, int line) {
    fprintf(stderr, "line:%d ", line);
    perror(str);
    exit(1);
}

int main(int argc, char *argv[]) {
    int lfd = 0;
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        sys_err("socket error", __LINE__);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int result;
    result = bind(lfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (result == -1) {
        sys_err("bind", __LINE__);
    }

    result = listen(lfd, 128);
    if (result == -1) {
        sys_err("listen", __LINE__);
    }

    struct sockaddr_in clit_addr;
    socklen_t clit_addr_len;

    clit_addr_len = sizeof(clit_addr);
    int cfd; // connect fd

    cfd = accept(lfd, (struct sockaddr *) &clit_addr, &clit_addr_len);
    // 阻塞监听客户端连接, 第二个参数是传出参数, 第三个参数是传入传出
    // 返回指向新的套接字的新的文件描述符, 是用来通信的
    if (cfd == -1) {
        sys_err("accept", __LINE__);
    } else {
        char *tmp = "连接建立成功!\n";
        printf("%s", tmp);

        char client_IP[1024];
        printf("client ip:%s port:%d\n",
               inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
               ntohs(clit_addr.sin_port));

        write(cfd, tmp, strlen(tmp));
    }

    char buf[BUFSIZ]; // BUFSIZ 8192

    while (1) {
        result = read(cfd, buf, sizeof(buf)); // 返回实际读到的字节数
        if (result == 0) {
            break;
        }
        write(STDOUT_FILENO, buf, result);

        for (int i = 0; i < result; ++i) {
            buf[i] = toupper(buf[i]);
        }

        write(cfd, buf, result); // 写回客户端
    }

    close(lfd);
    close(cfd);

    return 0;
}
