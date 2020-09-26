#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>

#include "../include/common.h"
#include "../include/wrap.h"
#include "../include/menu.h"
#include "../include/md5.h"

char has_login_id[10];
UserData *userdataHead;
Box *box;  // 消息盒子
Pack *recv_pack;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    int client_sockfd;
    client_sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_port = htons(PORT);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    Connect(client_sockfd, (struct sockaddr *) &addr, sizeof(addr));
    printf("和服务器连接成功!\n");
    pthread_t pid;
    pthread_create(&pid, NULL, response_thread, (void *) &client_sockfd);
    sleep(1);

//    initLinkList(); // 初始化用户数据链表

    int is_login;
    while (1) {
        puts("\033[2J"); // 清屏操作
        start_menu();
        char input = getchar();
        if (input == '1') {
            puts("\033[2J");
            login(client_sockfd, &is_login);
            getchar();
            if (is_login == 1) { // 如果登陆成功则退出循环
                break;
            }
        } else if (input == '2') {
            puts("\033[2J");
            register1(client_sockfd);
            getchar();
        } else if (input == '3') {
            puts("\033[2J");
            find_my_passwd(client_sockfd);
            getchar();
        } else if (input == '4') {
            puts("\033[2J");
            change_password(client_sockfd);
            sleep(2);
        } else if (input == '5') {
            puts("\033[2J");
            exit_function(client_sockfd);
        } else {
            continue;
        }
    }

    while (1) {
        puts("\033[2J"); // 清屏操作
        secondary_menu();
        char input = getchar();
        if (input == '1') {                 // 好友管理
            puts("\033[2J");
            friends_management_menu();
            int choice;
            scanf("%d", &choice);
            switch (choice) {
                case ADD_FRIEND: {
                    add_friend(client_sockfd);
                    getchar();
                    break;
                }
                case DEL_FRIEND: {
                    del_friend(client_sockfd);
                    break;
                }
//                case SEARCH_FRIEND: {
//                    search_friend(client_sockfd);
//                    break;
//                }
                case VIEW_FRIEND_LIST: {
                    view_friend_list(client_sockfd);
                    break;
                }
                case VIEW_FRIEND_CHAT_MSG: {

                    break;
                }
                case ADD_BLACKLIST: {
                    add_blacklist(client_sockfd);
                    break;
                }
                case DEL_BLACKLIST: {
                    del_blacklist(client_sockfd);
                    break;
                }
                case CHAT_WITH_FRIEND: {
                    chat_with_friend(client_sockfd);
                    break;
                }

                case VIEW_MESSAGE: { // 查看消息
                    view_messaeg(client_sockfd);
                }

                default: {
                    continue;
                }
            }
        } else if (input == '2') {          // 群管理
            puts("\033[2J");
            group_management_menu();
            int choice;
            scanf("%d", &choice);
            switch (choice) {
                case CREATE_GROUP: {
                    create_group(client_sockfd);
                    getchar();
                    break;
                }
                case EXIT_GROUP: {
                    exit_group(client_sockfd);
                    break;
                }
                case JOIN_GROUP: {
                    view_friend_list(client_sockfd);
                    break;
                }
                case VIEW_GROUP_LIST: {
                    view_group_list(client_sockfd);
                    break;
                }
                case VIEW_GROUP_MEMBER: {
                    add_blacklist(client_sockfd);
                    break;
                }
                case VIEW_GROUP_CHAT_MSG: {
                    del_blacklist(client_sockfd);
                    break;
                }
                case SET_GROUP_MANAGER: {
                    chat_with_friend(client_sockfd);
                    break;
                }

                case KICK_PEOPLE: { // 查看消息
                    view_messaeg(client_sockfd);
                }
                default: {
                    continue;
                }
            }
        } else if (input == '3') {          // 文件管理
            puts("\033[2J");
            file_management_menu();


            sleep(2);
        } else if (input == '4') {          // 退出
            puts("\033[2J");

            exit_function(client_sockfd);
            exit(0);

        } else {
            continue;
        }
    }
}


void *response_thread(void *arg) {
    pthread_detach(pthread_self());
    int client_sockfd = *(int *) arg;

    recv_pack = (Pack *) malloc(sizeof(Pack));
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    box = (Box *) malloc(sizeof(Box));
    box->friend_request_num = 0;

    while (1) {
        pthread_mutex_lock(&mutex);
        Recv(client_sockfd, recv_pack, sizeof(Pack), MSG_WAITALL);
        pthread_mutex_unlock(&mutex);
        sleep(1);
        switch (recv_pack->type) {
            case ADD_FRIEND: {
                printf("你收到了一条添加好友消息\n");
                box->friend_request_num++;
                strcpy(box->id[box->friend_request_num - 1], recv_pack->id);      // 保存原客户端id
                strcpy(box->name[box->friend_request_num - 1], recv_pack->name);  // 保存name
                strcpy(box->client_msg[box->friend_request_num - 1], recv_pack->server_msg); // 保存自己id
                getchar();
                break;
            }
            case REAL_CHAT: {
                printf("你收到了一条私聊消息\n");
                printf("账号为%s昵称为%s的好友:%s\n", recv_pack->id, recv_pack->name, recv_pack->message);
                getchar();
                break;
            }
        }
    }
}

void login(int client_sockfd, int *is_login) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    userdataHead = (UserData *) malloc(sizeof(UserData));   /////// 注意


    send_pack->type = LOGIN;

    printf("请输入账号:\n");
    scanf("%s", send_pack->id);
    printf("请输入密码:\n");

    char passwd[30];

    char c;
    int i = 0;
    setbuf(stdin, NULL); // 清空缓冲区
    while ((c = getch()) != '\r') {   /////////// \r  TODO
        if (c == '\b' && i > 0) {  //删除功能
            i--;
            printf("\b \b");
        }

        if (isprint(c)) {
            passwd[i++] = c;
            putchar('*');
        }
    }
    putchar('\n');
    passwd[i] = '\0';

    strcpy(send_pack->passwd, passwd);
    send_pack->client_sockfd = client_sockfd; // 设置sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("登录成功\n");
        strcpy(has_login_id, send_pack->id);
        *is_login = 1;
    } else if (strcmp(recv_pack->respond, "fail") == 0) {
        printf("登陆失败, 账号或密码错误\n");
        *is_login = 0;
        return;
    } else if (strcmp(recv_pack->respond, "has_login") == 0) {
        printf("登陆失败, 该账号已经登录\n");
        *is_login = 0;
        return;
    }

    Recv(client_sockfd, userdataHead, sizeof(UserData), MSG_WAITALL);
    //// 处理添加好友请求
    printf("在你离线期间, 你有%d条添加好友请求\n", userdataHead->friend_request_num);
    while (userdataHead->friend_request_num--) { // 1 -- = 0   2 -- = 1  1 -- = 0
        printf("%s\n", userdataHead->server_msg[userdataHead->friend_request_num]);
        printf("请输入1为接受, 0为拒绝:");
        int choice = 0;
        while (1) {
            scanf("%d", &choice);
            if (choice == 1) {
                send_pack->type = HANDLE_ADD_FRIEND_REQUESTS;

                send_pack->client_sockfd = client_sockfd;
                strcpy(send_pack->id, userdataHead->id); // 保存自己的id
                strcpy(send_pack->client_msg, userdataHead->friend_id[userdataHead->friend_request_num]); // 保存原客户端的id

                Send(client_sockfd, send_pack, sizeof(Pack), 0);
                break;
            } else if (choice == 0) {
                break;
            } else {
                printf("请重新输入\n");
                continue;
            }
        }
    }
    //// 处理私聊
    printf("在你离线期间, 你有%d条私聊请求\n", userdataHead->friend_chat_num);
    while (userdataHead->friend_chat_num--) { // 1 -- = 0   2 -- = 1  1 -- = 0
        printf("%s\n", userdataHead->server_msg[userdataHead->friend_chat_num]);
    }
    pthread_mutex_unlock(&mutex);
}

void register1(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = REGISTER;

    printf("请设置昵称:\n");
    scanf("%s", send_pack->name);
    printf("请设置密码:\n");
    char passwd[30];

    char c;
    int i = 0;
    setbuf(stdin, NULL); // 清空缓冲区
    while ((c = getch()) != '\r') {
        if (c == '\b' && i > 0) {  //删除功能
            i--;
            printf("\b \b");
        }

        if (isprint(c)) {
            passwd[i++] = c;
            putchar('*');
        }
    }
    putchar('\n');
    passwd[i] = '\0';

    char encrypt[30];
    strcpy(encrypt, passwd);
    strcpy(send_pack->passwd, passwd);

    char s[32];
    MD5Change(s, encrypt);

    printf("请设置密保问题\n");
    scanf("%s", send_pack->security_question);
    printf("请设置密保答案\n");
    scanf("%s", send_pack->security_answer);
    send_pack->online = 0; // 不在线
    send_pack->client_sockfd = client_sockfd; // 设置sockfd
    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("注册成功\n");
        strcpy(has_login_id, recv_pack->id);
        printf("你的账号是:%s\n", recv_pack->id);
        printf("请输入回车键继续...\n");
        getchar();
    } else {
        printf("注册失败\n");
    }
    pthread_mutex_unlock(&mutex);
}

void find_my_passwd(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = FIND_MY_PASSWD;
    printf("请输入账号:\n");
    scanf("%s", send_pack->id);

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("该账号的密保问题是:%s\n", recv_pack->security_question);
        printf("请输入密保答案:\n");
        char answer[30];
        scanf("%s", answer);
        getchar();
        if (strcmp(answer, recv_pack->security_answer) == 0) {
            printf("密保正确\n你的原密码是:%s\n", recv_pack->passwd);
        } else {
            printf("密保不正确\n");
        }
    } else {
        printf("该账号不存在\n");
    }
    pthread_mutex_unlock(&mutex);
}

void change_password(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->flag = 0;
    send_pack->type = CHANGE_PASSWORD;
    printf("请输入账号:\n");
    scanf("%s", send_pack->id);

    printf("请输入原密码:\n");

    char passwd[30];

    char c;
    int i = 0;
    setbuf(stdin, NULL); // 清空缓冲区
    while ((c = getch()) != '\r') {
        if (c == '\b' && i > 0) {  //删除功能
            i--;
            printf("\b \b");
        }

        if (isprint(c)) {
            passwd[i++] = c;
            putchar('*');
        }
    }
    putchar('\n');
    passwd[i] = '\0';


    strcpy(send_pack->passwd, passwd);
    send_pack->client_sockfd = client_sockfd;

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("该账号的密保问题是:%s\n", recv_pack->security_question);
        printf("请输入密保答案:\n");
        char answer[30];
        scanf("%s", answer);
        if (strcmp(answer, recv_pack->security_answer) == 0) {
            printf("密保正确\n请输入修改后的密码:\n");
            scanf("%s", send_pack->client_msg);
            send_pack->flag = 1;
            Send(client_sockfd, send_pack, sizeof(Pack), 0);
            printf("密码修改成功\n");
        } else {
            printf("密保不正确\n");
        }
    } else {
        printf("该账号不存在\n");
    }
    pthread_mutex_unlock(&mutex);
}

void exit_function(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    strcpy(send_pack->id, has_login_id); // 设置id
    send_pack->type = EXIT;
    send_pack->client_sockfd = client_sockfd;

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    sleep(1);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("退出成功\n");
    } else {
        printf("退出成功\n");
    }
    Close(client_sockfd);
    exit(0);
}


void add_friend(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = ADD_FRIEND;
    printf("请输入要添加为好友的用户账号:\n");
    scanf("%s", send_pack->id);
    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("申请添加成功, 请等待该用户处理\n");
        recv_pack->type = NOTUSE;
    } else {
        printf("该好友不存在或已经是好友或者你要自己加自己为好友\n");
    }
    pthread_mutex_unlock(&mutex);
    getchar();
}


void view_messaeg(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));
    printf("你有%d条好友请求\n", box->friend_request_num);
    while (box->friend_request_num--) { // 1-- = 0  2-- = 1 1-- = 0
        printf("账号为%s昵称为%s的用户想添加你为好友\n",
               box->id[box->friend_request_num], box->name[box->friend_request_num]);
        printf("请输入1为接受, 0为拒绝\n");
        int choice;
        scanf("%d", &choice);
        while (1) {
            if (choice == 1) {
                send_pack->type = HANDLE_ADD_FRIEND_REQUESTS;

                send_pack->client_sockfd = client_sockfd;
                strcpy(send_pack->id, box->client_msg[box->friend_request_num]); // 保存自己的id
                strcpy(send_pack->client_msg, box->id[box->friend_request_num]); // 保存原客户端的id

                Send(client_sockfd, send_pack, sizeof(Pack), 0);
                break;
            } else if (choice == 0) {
                break;
            } else {
                printf("输入有误, 请重新输入\n");
                getchar();
                continue;
            }
        }
    }
}

void del_friend(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = DEL_FRIEND;
    printf("请输入要删除好友的用户账号:\n");
    scanf("%s", send_pack->id);
    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("删除好友成功\n");
        recv_pack->type = NOTUSE;
    } else {
        printf("该好友不存在或你们不是好友\n");
    }
    pthread_mutex_unlock(&mutex);
    getchar();
}

void view_friend_list(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = VIEW_FRIEND_LIST;

    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("查询好友成功\n");
        Recv(client_sockfd, box, sizeof(Box), MSG_WAITALL);
        printf("你的好友列表为:\n");

        while (box->num--) { // 2-- = 1   1-- = 0
            printf("账号:%s 是否在线%d\n", box->id[box->num], box->online[box->num]);
        }

        recv_pack->type = NOTUSE;
    } else {
        printf("没有好友\n");
    }
    pthread_mutex_unlock(&mutex);
    getchar();
}

void add_blacklist(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = ADD_BLACKLIST;
    printf("请输入你要添加黑名单的好友账号:");
    scanf("%s", send_pack->id);
    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("添加黑名单成功\n");
    } else {
        printf("该账号不存在或不是你的好友\n");
    }
    pthread_mutex_unlock(&mutex);
}

void del_blacklist(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = DEL_BLACKLIST;
    printf("请输入你要删除黑名单的好友账号:");
    scanf("%s", send_pack->id);
    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("删除黑名单成功\n");
    } else {
        printf("该账号不存在或不是你的好友\n");
    }
    pthread_mutex_unlock(&mutex);
}

void chat_with_friend(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = CHAT_WITH_FRIEND;

    printf("请输入你要私聊的好友账号:");
    scanf("%s", send_pack->id);

    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("该账号存在, 且是你的好友, 且没有被拉黑\n");

        while (1) {
            printf("请输入要发送的消息:(输入!q结束)\n");
            scanf("%s", send_pack->message);
            if (strcmp(send_pack->message, "!q") == 0) {
                break;
            }
            send_pack->type = REAL_CHAT;
            Send(client_sockfd, send_pack, sizeof(Pack), 0);
        }
    } else {
        printf("该账号不存在或不是你的好友, 或者你被拉黑了\n");
    }
    pthread_mutex_unlock(&mutex);
}

void create_group(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = CREATE_GROUP;

    printf("请输入要创建的群名:\n");
    scanf("%s", send_pack->group_name);
    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("创建群成功\n");
    } else {
        printf("创建群失败\n");
    }
    pthread_mutex_unlock(&mutex);
}

void exit_group(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = EXIT_GROUP;

    printf("请输入要退出的群号:\n");
    scanf("%s", send_pack->group_id);
    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("成功\n");
    } else {
        printf("失败\n");
    }
    pthread_mutex_unlock(&mutex);
}

void view_group_list(int client_sockfd) {
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));

    send_pack->type = VIEW_GROUP_LIST;

    send_pack->client_sockfd = client_sockfd; // 设置client_sockfd

    Send(client_sockfd, send_pack, sizeof(Pack), 0);

    pthread_mutex_lock(&mutex);
    if (strcmp(recv_pack->respond, "success") == 0) {
        printf("成功\n");
    } else {
        printf("失败\n");
    }
    pthread_mutex_unlock(&mutex);
}


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


void initLinkList() {
    userdataHead = (UserData *) malloc(sizeof(UserData));
    userdataHead->next = NULL;
}