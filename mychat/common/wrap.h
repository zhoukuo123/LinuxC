#ifndef LINUXC_WRAP_H
#define LINUXC_WRAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>

void perr_exit(const char *string);

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);

int Bind(int fd, const struct sockaddr *sa, socklen_t salen);

int Connect(int fd, const struct sockaddr *sa, socklen_t salen);

int Listen(int fd, int backlog);

int Socket(int family, int type, int protocol);

ssize_t Read(int fd, void *ptr, size_t nbytes);

ssize_t Write(int fd, const void *ptr, size_t nbytes);

int Close(int fd);

ssize_t Readn(int fd, void *ptr, size_t n);

ssize_t Writen(int fd, const void *vptr, size_t n);

static ssize_t my_read(int fd, char *ptr);

ssize_t Readline(int fd, void *vptr, size_t maxlen);

#endif //LINUXC_WRAP_H
