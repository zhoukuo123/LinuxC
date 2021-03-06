#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void sys_err(char *str) {
    perror(str);
    exit(1);
}

int main() {
    if (access("myfifo", F_OK) < 0) {
        mkfifo("myfifo", 0644);
    }

    int fd;
    char buf[1024] = "HelloWorld\n";
    fd = open("myfifo", O_WRONLY);
    if (fd < 0) {
        sys_err("open");
    }
    write(fd, buf, strlen(buf));
    close(fd);
    return 0;
}