#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

void my_err(const char *err_string, int line) {
    fprintf(stderr, "line:%d ", line);
    perror(err_string);
    exit(1);
}

int my_read(int fd) {
    int len, ret, i;
    char read_buf[64];

    // 获取文件长度, 并存于len,    
    if (lseek(fd, 0, SEEK_END) == -1) {
        my_err("lseek", __LINE__);
    }
    if ((len = lseek(fd, 0, SEEK_CUR)) == -1) {
        my_err("lseek", __LINE__);
    }

    // 并保持文件读写指针在文件开始处, 方便读取
    if ((lseek(fd, 0, SEEK_SET)) == -1) {
        my_err("lseek", __LINE__);
    }
    printf("len:%d\n", len);
    if ((ret = read(fd, read_buf, len) != len)) {
        my_err("read", __LINE__);
    }

    // 打印数据
    for (i = 0; i < len; i++) {
        printf("%c", read_buf[i]);
    }
    printf("\n");
    return ret;
}

int main() {
    int fd;
    char write_buf[32] = "Hello World!";

    if ((fd = open("test.c", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1) {
        my_err("open", __LINE__);
    } else {
        printf("Creat file success\n");
    }

    if (write(fd, write_buf, strlen(write_buf)) != (ssize_t) strlen(write_buf)) {
        my_err("write", __LINE__);
    }
    my_read(fd);

    // 演示文件间隔
    if (lseek(fd, 10, SEEK_END) == -1) {
        my_err("lseek", __LINE__);
    }
    if (write(fd, write_buf, strlen(write_buf)) != (ssize_t) strlen(write_buf)) {
        my_err("write", __LINE__);
    }
    my_read(fd);
    close(fd);
    return 0;
}

