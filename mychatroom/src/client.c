#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <wrap.h>

#include "../include/server.h"

PACK *send_pack;
PACK *recv_pack;
BOX *box;
FRIEND *friend_list;
MESSAGE *message;
//GROUP_MESSAGE *group_message;
GROUP *member_list;
GROUP_G *group_list;
FLE *file;
int sing;

pthread_mutex_t mutex;
pthread_mutex_t mutex_cli;
pthread_cond_t cond_cli;

int getch() {
    struct termios tm, tm_old;
    int fd = STDIN_FILENO, c;
    if (tcgetattr(fd, &tm) < 0)
        return -1;
    tm_old = tm;
    cfmakeraw(&tm);
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
        return -1;
    c = fgetc(stdin);
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
        return -1;
    if (c == 3)
        exit(1);
    //按Ctrl+C结束退出
    return c;
}

void *recv_pack_thread(void *sock_fd) { // 负责接收从服务端发回来的数据包(接受线程)
    group_list = (GROUP_G *) malloc(sizeof(GROUP_G));
    member_list = (GROUP *) malloc(sizeof(GROUP));
    friend_list = (FRIEND *) malloc(sizeof(FRIEND));
    box = (BOX *) malloc(sizeof(BOX));
    recv_pack = (PACK *) malloc(sizeof(PACK));
    message = (MESSAGE *) malloc(sizeof(MESSAGE));
//    group_message = (GROUP_MESSAGE *) malloc(sizeof(GROUP_MESSAGE));
    file = (FLE *) malloc(sizeof(FLE));
    file->have = 0;
    while (1) {
        memset(recv_pack, 0, sizeof(PACK));
        Recv(*(int *) sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
        switch (recv_pack->type) {
            case EXIT: {
                printf("退出成功\n");
                pthread_exit(0);
            }
            case FIND_MY_PASSWORD: {
                memset(send_pack->data.respond, 0, sizeof(send_pack->data.respond));
                strcpy(send_pack->data.respond, recv_pack->data.respond);
                pthread_mutex_lock(&mutex_cli);
                pthread_cond_signal(&cond_cli);
                pthread_mutex_unlock(&mutex_cli);
                break;
            }
            case SEND_FILE: { // 发文件
                pthread_mutex_lock(&mutex_cli);
                sing = 1;
                pthread_cond_signal(&cond_cli);
                pthread_mutex_unlock(&mutex_cli);
                break;
            }
            case RECV_FILE: { // 接受文件
                memset(file, 0, sizeof(file));
                file->send_account = recv_pack->data.send_account;
                strcpy(file->send_nickname, recv_pack->data.send_user);
                strcpy(file->filename, recv_pack->data.respond);
                file->have = 1;
                printf("账号%d\t昵称%s\t的好友给你发送了一个%s文件快去接收吧\n", file->send_account, file->send_nickname, file->filename);
                break;
            }
            case OK_FILE: {
                pthread_mutex_lock(&mutex_cli);
                sing = 1;
                pthread_cond_signal(&cond_cli);
                pthread_mutex_unlock(&mutex_cli);
                break;
            }
            case READ_FILE: { // 读文件
                memset(send_pack->data.respond, 0, sizeof(send_pack->data.respond));
                strcpy(send_pack->data.respond, recv_pack->data.respond);
                int fd = open("3", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
                write(fd, recv_pack->data.password, 1023);
                close(fd);
                pthread_mutex_lock(&mutex_cli);
                sing = 1;
                pthread_cond_signal(&cond_cli);
                pthread_mutex_unlock(&mutex_cli);
                break;
            }
            case ACCOUNT_ERROR: { // 账号错误
                memset(send_pack->data.respond, 0, sizeof(send_pack->data.respond));
                strcpy(send_pack->data.respond, recv_pack->data.respond);
                pthread_mutex_lock(&mutex_cli);
                sing = 1;
                pthread_cond_signal(&cond_cli);
                pthread_mutex_unlock(&mutex_cli);
                break;
            }
            case FRIEND_REQUEST: { // 好友请求
                pthread_mutex_lock(&mutex_cli);
                box->plz_account[box->friend_number] = recv_pack->data.send_account;
                strcpy(box->write_buff[box->friend_number], recv_pack->data.readbuff);
                box->friend_number++;
                printf("消息盒子中来了一条好友请求\n");
                pthread_mutex_unlock(&mutex_cli);
                break;
            }
            case recv_group_messaage: { // 接受群消息
                if (recv_pack->data.recv_account == send_pack->data.recv_account) {
                    printf("群号%d 群名%s 账号%d 昵称%s:\t%s\n", recv_pack->data.recv_account, recv_pack->data.recv_user,
                           recv_pack->data.send_account, recv_pack->data.send_user, recv_pack->data.readbuff);
                } else {
                    printf("消息盒子里来了一条群消息!!\n");
                    box->group_account[box->number] = recv_pack->data.recv_account;
                    box->send_account1[box->number] = recv_pack->data.send_account;
                    strcpy(box->message[box->number++], recv_pack->data.password);
                }
                break;
            }
            case RECV_FRIEND_MESSAGE: { // 接受好友消息
                if (recv_pack->data.send_account == send_pack->data.recv_account) {
                    printf("账号为%d昵称为%s的好友说:\t%s\n", recv_pack->data.send_account, recv_pack->data.send_user,
                           recv_pack->data.readbuff);
                } else if (strcmp(recv_pack->data.respond, "concern") == 0) {
                    printf("来自特别关心%d昵称%s的好友说:\t%s\n", recv_pack->data.send_account, recv_pack->data.send_user,
                           recv_pack->data.readbuff);
                } else {
                    box->send_account[box->talk_number] = recv_pack->data.send_account;
                    strcpy(box->read_buff[box->talk_number++], recv_pack->data.readbuff);
                    printf("消息盒子里来了一条好友消息!\n");
                }
                break;
            }
        }
    }
}


int main() {
    int sock_fd;
    sing = 0;
    pthread_mutex_init(&mutex_cli, NULL);
    pthread_cond_init(&cond_cli, NULL);
    struct sockaddr_in server;
    sock_fd = Socket(AF_INET, SOCK_STREAM, 0);
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "47.94.14.45", &server.sin_addr.s_addr);
    Connect(sock_fd, (struct sockaddr *) &server, sizeof(struct sockaddr_in));
    pthread_t pid1;

    pthread_create(&pid1, NULL, recv_pack_thread, (void *) &sock_fd);


    int pick;
    char ch;
    int i;
    send_pack = (PACK *) malloc(sizeof(PACK));

    group_list = (GROUP_G *) malloc(sizeof(GROUP_G));
    member_list = (GROUP *) malloc(sizeof(GROUP));
    friend_list = (FRIEND *) malloc(sizeof(FRIEND));
    box = (BOX *) malloc(sizeof(BOX));
    recv_pack = (PACK *) malloc(sizeof(PACK));
    message = (MESSAGE *) malloc(sizeof(MESSAGE));
//    group_message = (GROUP_MESSAGE *) malloc(sizeof(GROUP_MESSAGE));
    file = (FLE *) malloc(sizeof(FLE));
    file->have = 0;

    while (1) {
        login_mune();
        printf("请选择你需要的功能:\n");
        scanf("%d", &pick);
        switch (pick) {
            case 1: {
                send_pack->type = LOGIN;
                printf("请输入账号:\n");
                scanf("%d", &send_pack->data.send_account);
                getchar();
                printf("请输入密码:\n");
                i = 0;
                while ((ch = getch()) != '\r') {
                    if (i < 1024 && isprint(ch)) {
                        send_pack->data.password[i++] = ch;
                        putchar('*');
                    } else if (i > 0 && ch == '\b') { // 如果输入的是退格键
                        --i;
                        printf("\b \b");
//                        putchar('\b');
//                        putchar(' ');
//                        putchar('\b');
                    }
                }
                send_pack->data.password[i] = '\0';
                printf("\n");
                memset(send_pack->data.respond, 0, sizeof(send_pack->data.respond));
                Send(sock_fd, send_pack, sizeof(PACK), 0);

                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                switch (recv_pack->type) {
                    case ACCOUNT_ERROR: {
                        printf("%s\n", recv_pack->data.respond);
                        printf("按下回车键继续...");
                        getchar();
                        continue;
                    }
                    case LOGIN: {
                        strcpy(send_pack->data.send_user, recv_pack->data.send_user); // 保存登陆账号的昵称
                        send_pack->data.send_fd = recv_pack->data.recv_fd; // 保存套接字
                        Recv(sock_fd, box, sizeof(BOX), MSG_WAITALL);
                        printf("离线期间消息盒子中有%d条消息,%d个好友请求,%d条群消息", box->talk_number, box->friend_number, box->number);
                        break;
                    }
                }
            }
            case 2: {
                send_pack->type = REGISTERED;
                printf("请输入昵称:\n");
                scanf("%s", send_pack->data.send_user);
                getchar();
                printf("请输入密码:\n");
                i = 0;
                while ((ch = getch()) != '\r') {
                    if (i < 1024 && isprint(ch)) {
                        send_pack->data.password[i++] = ch;
                        putchar('*');
                    } else if (i > 0 && ch == '\b') { // 如果输入的是退格键
                        --i;
                        printf("\b \b");
//                        putchar('\b');
//                        putchar(' ');
//                        putchar('\b');
                    }
                }
                send_pack->data.password[i] = '\0';
                printf("\n");
                scanf("请输入你的密保:%s", send_pack->data.password_security);
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                send_pack->data.send_account = recv_pack->data.send_account; // 保存客户端账号
                printf("注册的账号为:%d\n", send_pack->data.send_account);
                printf("按下回车键继续...");
                getchar();
                continue;
            }
            case 3: {
                send_pack->type = FIND_MY_PASSWORD;
                printf("请输入你的账号:\n");
                scanf("%d", &send_pack->data.send_account);
                getchar();
                printf("请输入你的密保:\n");
                scanf("%s", send_pack->data.password_security);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                printf("%s\n", recv_pack->data.respond);
                printf("按下回车键继续.......");
                getchar();
                continue;
            }
            case 4: {
                send_pack->type = EXIT;
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                pthread_exit(0);
            }
            default: {
                printf("输入有误\n");
                printf("输入回车继续...");
                getchar();
                getchar();
                break;
            }
        }
        if (pick > 4 || pick < 1) {
            continue;
        } else if (pick == 1) {
            printf("按下回车键继续...\n");
            getchar();
            break; // 退出循环
        }
    }

    while (1) {
        mune();
        printf("\n");
        scanf("%d", &pick);
        getchar();
        switch (pick) {
            case 1: {
                send_pack->type = CHANGE_PASSWORD;
                printf("请输入原始密码:\n");
                i = 0;
                while ((ch = getch()) != '\r') {
                    if (i < 1024 && isprint(ch)) {
                        send_pack->data.password[i++] = ch;
                        putchar('*');
                    } else if (i > 0 && ch == '\b') { // 如果输入的是退格键
                        --i;
                        printf("\b \b");
//                        putchar('\b');
//                        putchar(' ');
//                        putchar('\b');
                    }
                }
                send_pack->data.password[i] = '\0';
                printf("\n");
                printf("请输入修改后的密码:\n");
                i = 0;
                while ((ch = getch()) != '\r') {
                    if (i < 1024 && isprint(ch)) {
                        send_pack->data.respond[i++] = ch;
                        putchar('*');
                    } else if (i > 0 && ch == '\b') { // 如果输入的是退格键
                        --i;
                        printf("\b \b");
//                        putchar('\b');
//                        putchar(' ');
//                        putchar('\b');
                    }
                }
                send_pack->data.respond[i] = '\0';
                printf("\n");
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("修改成功\n");
                    printf("按下回车继续......\n");
                    getchar();
                } else {
                    printf("修改失败\n");
                    printf("按下回车继续.......\n");
                    getchar();
                }
                break;
            }
            case 2: {
                send_pack->type = ADD_FRIEND;
                printf("请输入你想添加好友的账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("发送成功, 等待对方确认.\n");
                    printf("按下回车键继续.......");
                    getchar();
                } else {
                    printf("好友账号不存在或者已经是你的好友\n");
                    printf("按下回车键继续.......");
                    getchar();
                }
                break;
            }
            case 3: {
                send_pack->type = DELETE_FRIEND;
                printf("请输入要删除的好友账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("删除成功\n");
                    printf("按下回车键继续..........");
                    getchar();
                } else {
                    printf("没有该好友\n");
                    printf("按下回车继续........");
                    getchar();
                }
                break;
            }
            case 4: {
                pthread_mutex_lock(&mutex_cli);
                send_pack->type = FRIEND_REQUEST;
                if (box->friend_number == 0) {
                    printf("消息盒子里没有好友请求!!\n");
                    printf("输入回车继续......");
                    pthread_mutex_unlock(&mutex);
                    getchar();
                } else {
                    for (int j = 0; j < box->friend_number; ++j) {
                        printf("%s\n", box->write_buff[j]);
                        send_pack->data.recv_account = box->plz_account[j];
                        printf("请选择: 1. 接受 2. 拒绝 3. 忽略\n");
                        scanf("%d", &pick);
                        getchar();
                        if (pick == 3 || pick == 2) {
                            continue;
                        } else if (pick == 1) {
                            Send(sock_fd, send_pack, sizeof(PACK), 0);
                        }
                    }
                    box->friend_number = 0;
                    printf("处理完成!!\n");
                    printf("回车键继续.......");
                    pthread_mutex_unlock(&mutex_cli);
                    getchar();
                }
                break;
            }
            case 5: {
                send_pack->type = BLACKLIST_FRIEND;
                printf("请输入你要添加入黑名单的好友账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("添加黑名单成功\n");
                    printf("按下回车继续....");
                    getchar();
                } else {
                    printf("对方不是你的好友或该账号不存在!!!\n");
                    printf("按下回车继续....");
                    getchar();
                }
                break;
            }
            case 6: {
                send_pack->type = CANCEL_BLACKLIST;
                printf("请输入你要取消黑名单的好友的账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();

                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("该好友取消黑名单成功\n");
                    printf("按下回车继续......");
                    getchar();
                } else {
                    printf("该好友不是你的好友或者该好友不在黑名单中\n");
                    printf("按下回车键继续.......");
                    getchar();
                }
                break;
            }
            case 7: {
                send_pack->type = CONCERN_FRIEND;
                printf("请输入你要特别关心的好友的账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("特别关心该好友成功\n");
                    printf("按下回车继续......");
                    getchar();
                } else {
                    printf("你没有这个好友或者该好友已经被特别关心了.\n");
                    printf("按下回车键继续.......");
                    getchar();
                }
                break;
            }
            case 8: {
                send_pack->type = CANCEL_CONCERN_FRIEND;
                printf("请输入你要取消特别关心的好友的账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("该的好友取消特别关心成功\n");
                    printf("按下回车继续......");
                    getchar();
                } else {
                    printf("你没有该好友或者该好友并没有被特别关心.\n");
                    printf("按下回车键继续.......");
                    getchar();
                }
                break;
            }
            case 9: {
                send_pack->type = VIEW_FRIEND_LIST;
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                memset(friend_list, 0, sizeof(FRIEND));
                Recv(sock_fd, friend_list, sizeof(FRIEND), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("好友列表:\n");
                    for (int j = 0; j < friend_list->friend_number; j++) {
                        printf("%d\t%-20s\t", friend_list->friend_account[j], friend_list->friend_nickname[j]);
                        if (friend_list->friend_state[j] == 1) {
                            printf("在线\n");
                        } else {
                            printf("不在线\n");
                        }
                    }
                } else {
                    printf("好友列表为空\n");
                }
                printf("按下回车键继续.......");
                getchar();
                break;
            }
            case 10: {
                send_pack->type = SEND_FRIEND_MESSAGE;
                printf("请输入你要聊天的好友账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                printf("开始与该好友聊天\n");
                while (1) {
                    scanf("%s", send_pack->data.readbuff);
                    if (strcmp(send_pack->data.readbuff, "@over") == 0) {
                        printf("与该好友聊天结束\n");
                        break;
                    }
                    Send(sock_fd, send_pack, sizeof(PACK), 0);
                    Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                    if (strcmp(recv_pack->data.respond, "fail") == 0) {
                        printf("没有该好友\n");
                        break;
                    }
                }
                send_pack->data.recv_account = 0;
                printf("按下回车键继续...");
                getchar();
                getchar();
                break;
            }
            case 11: {
                if (box->talk_number == 0) {
                    printf("你没有未看的好友消息\n");
                } else {
                    for (int j = 0; j < box->talk_number; ++j) {
                        printf("账号%d:%s\n", box->send_account[j], box->read_buff[j]);
                    }
                    box->talk_number = 0;
                }
                printf("按下回车继续......");
                getchar();
                break;
            }
            case 12: {
                send_pack->type = SEND_GROUP_MESSAGE;
                printf("请输入你要聊天的群号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                printf("开始发言:\n");
                while (1) {
                    scanf("%s", send_pack->data.readbuff);
                    getchar();
                    if (strcmp(send_pack->data.readbuff, "@over") == 0) {
                        printf("在群%d的发言结束\n", send_pack->data.recv_account);
                        break;
                    }
                    Send(sock_fd, send_pack, sizeof(PACK), 0);
                    Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                    if (strcmp(recv_pack->data.respond, "fail") == 0) {
                        printf("没有该群\n");
                        break;
                    }
                }
                send_pack->data.recv_account = 0;
                printf("按下回车键个继续...");
                getchar();
                break;
            }
            case 13: {
                send_pack->type = ADD_GROUP;
                printf("请输入你想加的群的群号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("加入群%d成功!\n", send_pack->data.recv_account);
                } else {
                    printf("没有群号为%d的群或你已加入该群\n", send_pack->data.recv_account);
                }
                printf("按下回车键继续......");
                getchar();
                break;
            }
            case 14: {
                send_pack->type = CREATE_GROUP;
                printf("请输入你要创建的群的昵称:\n");
                scanf("%s", send_pack->data.recv_user);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                strcpy(send_pack->data.recv_user, recv_pack->data.recv_user); // 保存群昵称
                send_pack->data.recv_account = recv_pack->data.recv_account; // 保存群号

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("创建群成功\n");
                    printf("群号%d\t群名%s\n", send_pack->data.recv_account, send_pack->data.recv_user);
                } else {
                    printf("创建失败\n");
                }
                printf("按下回车键继续.....");
                getchar();
                break;
            }
            case 15: {
                send_pack->type = EXIT_GROUP;
                printf("请输入你要退出的群的群号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("退出该群成功\n");
                } else {
                    printf("你没有加入这个群\n");
                }
                printf("按下回车键继续......");
                getchar();
                break;
            }
            case 16: {
                send_pack->type = SET_ADMIN;
                printf("请输入要设置的群号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                printf("请输入要设置为管理员的群成员的账号:\n");
                scanf("%s", send_pack->data.writebuff);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("该成员被设置为管理员成功\n");
                } else {
                    printf("该群里面没有该成员或者你不是该群群主或者你不在该群\n");
                }
                printf("按下回车继续.......");
                getchar();
                break;
            }
            case 17: {
                send_pack->type = CANCEL_ADMIN;
                printf("请输入群号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                printf("输入要被取消管理员的群员账号:\n");
                scanf("%s", send_pack->data.readbuff);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("取消成功\n");
                } else {
                    printf("群%d里面没有%s这个成员或你不是该群群主\n", send_pack->data.recv_account, send_pack->data.readbuff);
                }
                printf("按下回车继续.......");
                getchar();
                break;
            }
            case 18: {
                send_pack->type = LOOK_GROUP_MEMBER;
                printf("输入你想查看的群:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);

                memset(member_list, 0, sizeof(GROUP));
                Recv(sock_fd, member_list, sizeof(GROUP), MSG_WAITALL);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (member_list->group_member_number == 0) {
                    printf("没有该群\n");
                    printf("按下回车键继续.......");
                    getchar();
                    break;
                }
                for (int j = 0; j < member_list->group_member_number; j++) {
                    printf("%d\t%-20s\t", member_list->group_member_account[j], member_list->group_member_nickname[j]);
                    if (member_list->group_member_state[j] == 1) {
                        printf("群主\n");
                    } else if (member_list->group_member_state[j] == 2) {
                        printf("管理员\n");
                    } else {
                        printf("普通群员\n");
                    }
                }
                printf("输入回车继续.......");
                getchar();
                break;
            }
            case 19: {
                struct stat buf;
                int fd;
                send_pack->type = SEND_FILE;
                printf("请输入你要发送文件的对象:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                printf("请输入你想要发送的文件的绝对路径(以'/'开头):\n");
                scanf("%s", send_pack->data.respond);
                getchar();
                if (lstat(send_pack->data.respond, &buf) < 0) {
                    printf("文件名输入错误\n");
                    printf("按下回车继续......");
                    getchar();
                    break;
                }
                if ((fd = open(send_pack->data.respond, O_RDONLY)) < 0) {
                    printf("文件打开失败!!\n");
                    printf("按下回车继续.....");
                    getchar();
                    break;
                }
                sing = 0;
                send_pack->data.cont = 0;
                memset(send_pack->data.password, 0, sizeof(send_pack->data.password));
                while (read(fd, send_pack->data.password, 1023) > 0) {
                    Send(sock_fd, send_pack, sizeof(PACK), 0);
                    printf("正在发送...%d\n", send_pack->data.cont);
                    memset(send_pack->data.password, 0, sizeof(send_pack->data.password));
                    pthread_mutex_lock(&mutex_cli);
                    while (sing == 0) {
                        pthread_cond_wait(&cond_cli, &mutex_cli);
                    }
                    pthread_mutex_unlock(&mutex_cli);
                    sing = 0;
                    send_pack->data.cont++;
                }
                close(fd);
                printf("$$$\n");
                send_pack->type = OK_FILE;
                sing = 0;
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                printf("&&\n");
                pthread_mutex_lock(&mutex_cli);
                while (sing == 0) {
                    pthread_cond_wait(&cond_cli, &mutex_cli);
                }
                pthread_mutex_unlock(&mutex_cli);
                sing = 0;
                printf("发送成功!!\n");
                printf("按下回车键继续.....");
                getchar();
                break;
            }
            case 20: {
                send_pack->type = VIEW_FRIEND_CHATMESSAGE;
                printf("请输入你要查看聊天记录的好友账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("查看聊天记录成功\n");
                }
                Recv(sock_fd, message, sizeof(MESSAGE), MSG_WAITALL);

                if (message->number == 0) {
                    printf("没有聊天记录\n");
                } else {
                    for (int j = 0; j < message->number; j++) {
                        printf("%d---->%d:\t%s\n", message->send_user[j], message->recv_user[j], message->message[j]);
                    }
                }
                printf("按下回车键继续.....");
                getchar();
                getchar();
                break;
            }
            case 21: {
                send_pack->type = DELETE_MESSAGE;
                printf("请输入要删除聊天记录的好友账号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("删除成功\n");
                } else {
                    printf("没有这个好友或与该好友没有聊天记录!\n");
                }
                printf("按下回车键继续.....");
                getchar();
                break;
            }
            case 22: { // 查看群消息
                if (box->number == 0) {
                    printf("消息盒子里面没有群消息!!");
                } else {
                    for (int j = 0; j < box->number; j++) {
                        printf("群号%d 发送人账号%d 消息内容:\t%s\n", box->group_account[j], box->send_account1[j],
                               box->message[j]);
                    }
                    box->number = 0;
                }
                printf("按下回车键继续......");
                getchar();
                break;
            }
            case 23: {
                send_pack->type = DELETE_GROUP_MEMBER;
                printf("输入群号:\n");
                scanf("%d", &send_pack->data.recv_account);
                getchar();
                printf("请输入你的要删除的成员的账号:\n");
                scanf("%s", send_pack->data.readbuff);
                getchar();
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);

                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("删除成功\n");
                } else {
                    printf("你没有权限\n");
                }
                printf("按下回车键继续.....");
                getchar();
                break;
            }
            case 24: {
                send_pack->type = LOOK_GROUP_LIST;
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                Recv(sock_fd, recv_pack, sizeof(PACK), MSG_WAITALL);
                if (strcmp(recv_pack->data.respond, "success") == 0) {
                    printf("查询成功\n");
                }
                memset(group_list, 0, sizeof(GROUP_G));
                Recv(sock_fd, group_list, sizeof(GROUP_G), MSG_WAITALL);

                if (group_list->number == 0) {
                    printf("你还没加群\n");
                    printf("按下回车键继续........");
                    getchar();
                    break;
                }
                for (int j = 0; j < group_list->number; ++j) {
                    printf("%d\t%-20s\t", group_list->group_account[j], group_list->group_name[j]);
                    if (group_list->group_state[j] == 1) {
                        printf("群主\n");
                    } else if (group_list->group_state[j] == 2) {
                        printf("管理员\n");
                    } else {
                        printf("普通群员\n");
                    }
                }
                printf("按下回车键继续......");
                getchar();
                break;
            }
            case 25: {
                send_pack->data.cont = 0;
                sing = 0;
                if (file->have == 0) {
                    printf("没有人给你发文件!!\n");
                    printf("按下回车键继续.....");
                    getchar();
                    break;
                } else {
                    printf("账号%d昵称%s的好友发来%s的文件\n", file->send_account, file->send_nickname, file->filename);
                    printf("请选择:\n1. 接收 2. 拒绝\n");
                    scanf("%d", &pick);
                    getchar();
                    if (pick == 1) {
                        while (1) {
                            send_pack->type = READ_FILE;
                            send_pack->data.recv_account = file->send_account;
                            memset(send_pack->data.respond, 0, sizeof(send_pack->data.respond));
                            strcpy(send_pack->data.respond, file->filename);
                            printf("%d\n", send_pack->data.cont);
                            Send(sock_fd, send_pack, sizeof(PACK), 0);
                            printf("开始接收文件.....");
                            pthread_mutex_lock(&mutex_cli);
                            while (sing == 0) {
                                pthread_cond_wait(&cond_cli, &mutex_cli);
                            }
                            pthread_mutex_unlock(&mutex_cli);
                            send_pack->data.cont++;
                            sing = 0;
                            if (strcmp(send_pack->data.respond, "ok") == 0) {
                                printf("文件接收完毕!!!\n");
                                printf("按下回车键继续......");
                                getchar();
                                break;
                            }
                        }
                    } else {
                        printf("你已拒绝接收这个文件!!\n");
                        printf("按下回车继续......\n");
                        getchar();
                    }
                }
                break;
            }
            case 26: {
                send_pack->type = EXIT;
                Send(sock_fd, send_pack, sizeof(PACK), 0);
                pthread_exit(0);
            }
        }
    }


    pthread_join(pid1, NULL);
//    pthread_join(pid2, NULL);
    return 0;
}

void login_mune() {
    printf("\033c");

    printf("                          ------------------------------------------------------------------------------                   \n\n\n\n\n\n");
    printf("                                                    1.   登录                                                                    \n\n\n");
    printf("                                                    2.   注册                                                                    \n\n\n");
    printf("                                                    3.   找回密码                                                                \n\n\n");
    printf("                                                    4.   退出                                                                \n\n\n");
    printf("                          ------------------------------------------------------------------------------                         \n\n\n");
    printf("请选择:\n");
}


void mune() {
    printf("\033c");
    printf("登陆成功!!!\n");

    printf("                           ---------------------------------------------------------------------------                       \n\n\n\n");
    printf("                                                    1.   修改密码                                                                \n");
    printf("                                                    2.   加好友                                                                  \n");
    printf("                                                    3.   删除好友                                                                \n");
    printf("                                                    4.   处理好友请求                                                            \n");
    printf("                                                    5.   拉黑好友                                                                \n");
    printf("                                                    6.   将好友从黑名单中取出                                                    \n");
    printf("                                                    7.   设置好友特别关心                                                        \n");
    printf("                                                    8.   取消好友特别关心                                                        \n");
    printf("                                                    9.   显示好友列表                                                            \n");
    printf("                                                    10.  私聊                                                                    \n");
    printf("                                                    11.  查看好友消息                                                            \n");
    printf("                                                    12.  群聊                                                                    \n");
    printf("                                                    13.  加群                                                                    \n");
    printf("                                                    14.  创建群聊                                                                \n");
    printf("                                                    15.  退出群                                                                  \n");
    printf("                                                    16.  设置管理员                                                              \n");
    printf("                                                    17.  删除管理员                                                              \n");
    printf("                                                    18.  查看群成员                                                              \n");
    printf("                                                    19.  传送文件                                                                \n");
    printf("                                                    20.  查看好友聊天记录                                                        \n");
    printf("                                                    21.  删除好友聊天记录                                                        \n");
    printf("                                                    22.  查看群消息                                                              \n");
    printf("                                                    23.  删除群成员(群踢人)                                                                   \n");
    printf("                                                    24.  查看已加入的群                                                          \n");
    printf("                                                    25.  接受文件                                                                \n");
    printf("                                                    26.  退出                                                                \n");
    printf("                           --------------------------------------------------------------------------                               ");
}