/**
 * epoll基于非阻塞I/O事件驱动
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define MAX_EVENTS 1024 // 监听上限数
#define BUFLEN 4096
#define SERV_PORT 8080

void recvdata(int fd, int events, void *arg);

void senddata(int fd, int evemts, void *arg);

/**
 * 描述就绪文件描述符相关信息
 */
struct myevent_s {
    int fd; // 要监听的文件描述符
    int events; // 对应的监听事件
    void *arg; // 泛型参数

    void (*call_back)(int fd, int events, void *arg); // 回调函数

    int status; // 是否在监听: 1->在红黑树上(监听), 0->不在(不监听)
    char buf[BUFLEN];
    int len;
    long last_active; // 记录每次加入红黑树 g_efd 的时间值
};

int g_efd; // 全局变量, 保存epoll_create返回的文件描述符
struct myevent_s g_events[MAX_EVENTS + 1]; // 自定义结构体类型数组 +1-->listen fd

// 将结构体myevent_s成员变量初始化
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg) {
    ev->fd = fd;
    ev->call_back = call_back;
    ev->events = 0;
    ev->arg = arg;
    ev->status = 0;
    memset(ev->buf, 0, sizeof(ev->buf));
    ev->len = 0;
    ev->last_active = time(NULL); // 调用eventset函数的时间

    return;
}

void eventadd(int efd, int events, struct myevent_s *ev) {
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events; // EPOLLIN 或 EPOLLOUT

    if (ev->status == 0) {
        op = EPOLL_CTL_ADD; // 将其加入红黑树 g_efd, 并将status置1
        ev->status = 1;
    }

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0) { // 实际添加/修改
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
    } else {
        printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
    }

    return;
}

// 从epoll监听的红黑树中删除一个文件描述符
void eventdel(int efd, struct myevent_s *ev) {
    struct epoll_event epv = {0, {0}};

    if (ev->status != 1) return; // 不在红黑树上

    // epv.data.ptr = ev;
    epv.data.ptr = NULL;
    ev->status = 0; // 修改状态
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv); // 从红黑树efd上将ev->fd摘除

    return;
}

// 当有文件描述符就绪, epoll返回, 调用该函数与客户端建立连接
void acceptconn(int lfd, int events, void *arg) {
    struct sockaddr_in cin; // client addr
    socklen_t len = sizeof(cin);
    int cfd, i;

    if ((cfd = accept(lfd, (struct sockaddr *) &cin, &len)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {
            // 暂时不做出错处理
        }
        printf("%s: accept, %s\n", __func__, strerror(errno));
        return;
    }

    do {
        for (i = 0; i < MAX_EVENTS; ++i) {
            if (g_events[i].status == 0) {
                break;
            }
        }

        if (i == MAX_EVENTS) {

        }
    }
}

// 创建socket, 初始化lfd
void initlistensocket(int efd, short port) {
    struct sockaddr_in sin;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK); // 将socket设为非阻塞

    memset(&sin, 0, sizeof(sin)); // bzero(&sin, sizeof(sin);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    bind(lfd, (struct sockaddr *) &sin, sizeof(sin)));

    listen(lfd, 20);

    // void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
    eventset(&g_events[MAX_EVENTS, lfd, acceptconn, &g_events[MAX_EVENTS]]);

    // void eventadd(int efd, int events, struct myevent_s *ev)
    eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);
}

int main(int argc, char *argv[]) {
    unsigned short port = SERV_PORT;

    if (argc == 2) {
        port = atoi(argv[1]); // 使用用户指定端口, 如未指定, 用默认端口
    }
    g_efd = epoll_create(MAX_EVENTS + 1); // 创建红黑树, 返回给全局 g_efd
    if (g_efd <= 0) {
        printf("create efd in %s err %s\n", __func__, strerror(errno));
    }

    initlistensocket(g_efd, port); // 初始化监听socket

    struct epoll_event events[MAX_EVENTS + 1]; // 保存已经满足就绪事件的文件描述符数组
    printf("server running:port[%d]\n", port);

    int checkpos = 0, i;
    while (1) {
        // 超时验证, 每次测试100个连接, 不测试listenfd 当客户端60秒内, 没有和服务器通信, 则关闭此客户端连接

        long now = time(NULL);
        for (i = 0; i < 100; i++, checkpos++) {
            if (checkpos == MAX_EVENTS) {
                checkpos = 0;
            }
            if (g_events[checkpos].status != 1) {
                continue;
            }

            long duration = now - g_events[checkpos].last_active;

            if (duration >= 60) {
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_efd, &g_events[checkpos]);
            }
        }

        // 监听红黑树g_efd, 将满足的事件的文件描述符加至events数组中, 1秒没有事件满足, 返回0
        int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
        if (nfd < 0) { // 异常
            printf("epoll_wait error, exit\n");
            break;
        }

        for (i = 0; i < nfd; ++i) {
            // 使用自定义结构体myevent_s类型指针, 接收联合体data的void *ptr成员
            struct myevent_s *ev = (struct myevent_s *) events[i].data.ptr;

            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) { // 读就绪事件
                ev->call_back(ev->fd, events[i].events, ev->arg);
                // lfd EPOLLIN
            }

            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) { // 写就绪事件
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }

    // 退出前释放所有资源
    return 0;
}
