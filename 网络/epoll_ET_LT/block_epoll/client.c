#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 10
#define SERV_PORT 9000
#define IP "localhost"

int main() {
    struct sockaddr_in servaddr;
    char buf[MAXLINE];
    int sockfd, i;
    char ch = 'a';

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

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
        write(sockfd, buf, sizeof(buf));
        sleep(5);
    }
    close(sockfd);
    return 0;
}
