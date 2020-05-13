#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

void sys_err(char *str) {
    perror(str);
    exit(1);
}

void do_sig_child(int signo) {
//    简单处理
//    while (waitpid(0, NULL, WNOHANG) != -1) {
//
//    }

    int status;
    pid_t pid;

    while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("child %d exit %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("child %d term signal %d\n", pid, WTERMSIG(status));
        }
    }
}

int main() {
    pid_t pid;
    for (int i = 0; i < 10; ++i) {
        if ((pid = fork()) == 0) {
            // in child
            break;
        } else if (pid < 0) {
            sys_err("fork");
        }
    }

    if (pid == 0) {
        int n = 8;
        while (n--) {
            printf("I am child, child id %d\n", getpid());
            sleep(1);
        }
    } else if (pid > 0) {
        struct sigaction act;
        act.sa_handler = do_sig_child;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGCHLD, &act, NULL);

        while (1) {
            printf("I am parent, parent id %d\n", getpid());
            sleep(1);
        }
    }

    return 0;

}