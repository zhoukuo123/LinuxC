#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <mysql/mysql.h>
#include <fcntl.h>
#include <signal.h>

#include "../include/common.h"
#include "../include/wrap.h"
#include "../include/md5.h"

MYSQL mysql;
MYSQL_RES *result;
MYSQL_RES *result2;
MYSQL_ROW row;
MYSQL_ROW row2;
int rowcount;
int rowcount2;

pthread_mutex_t mutex;
pthread_cond_t cond;

UserData *userdataHead;

int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    int listen_fd = init_server();

    //首先连接数据库
    mysql_init(&mysql);
    if (mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "chatroom", 3306, NULL, 0) == NULL) {
        printf("数据库有问题, 导致连接不上了\n");
        mysql_close(&mysql);
        printf("服务器退出\n");
        return 1;
    } else {
        printf("连接数据库成功\n");

        // 处理服务器日志
        int fd = open("/home/linux/CLionProjects/LinuxC/mychatroom/src/server_log.txt", O_RDWR);
        char buf[30] = "连接数据库成功\n";
        write(fd, buf, strlen(buf));
        close(fd);
    }

    // 处理服务器日志
    int fd = open("/home/linux/CLionProjects/LinuxC/mychatroom/src/server_log.txt", O_APPEND | O_RDWR);
    char buf[30] = "服务器启动成功\n";
    write(fd, buf, strlen(buf));
    close(fd);

    struct epoll_event ev;

    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;

    struct epoll_event events[20];
    int nfds, i;
    int maxfds = 20;

    int epfd = epoll_create(maxfds);

    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

//    initLinkList(); // 初始化用户数据链表

    while (1) {
        nfds = epoll_wait(epfd, events, 20, -1);
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) { // 有客户端连接请求
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int connfd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
                printf("客户端与服务器成功连接\n");

                // 处理服务器日志
                int fd2 = open("/home/linux/CLionProjects/LinuxC/mychatroom/src/server_log.txt", O_APPEND | O_RDWR);
                char buf2[30] = "服务器启动成功\n";
                write(fd2, buf2, strlen(buf2));
                close(fd2);

                // 把新的用于服务器和客户端通信的文件描述符挂在树上
                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            } else if (events[i].events & EPOLLIN) {        // 有客户端发包
                Pack *recv_pack = (Pack *) malloc(sizeof(Pack));
                Recv(events[i].data.fd, recv_pack, sizeof(Pack), MSG_WAITALL);
                recv_pack->server_sockfd = events[i].data.fd;
                pthread_t pid;
                pthread_create(&pid, NULL, handle_thread, (void *) recv_pack);
            }
        }
    }


}

void *handle_thread(void *arg) {
    pthread_detach(pthread_self());
    Pack *recv_pack = (Pack *) arg;
    Pack *send_pack = (Pack *) malloc(sizeof(Pack));
    switch (recv_pack->type) {
        case LOGIN: {
            char sql_sentence[500];

            sprintf(sql_sentence,
                    "select * from account where id = '%s' and passwd = '%s'",
                    recv_pack->id, recv_pack->passwd);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            int flag = 0;
            int failure = 0;
            UserData *cur = userdataHead;
            UserData *tmp = (UserData *) malloc(sizeof(UserData));
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
                failure = 1;
            } else {
                ////// 判断是否已经在线
                row = mysql_fetch_row(result); // 拿到那一行数据
                if (strcmp(row[5], "1") == 0) {
                    strcpy(send_pack->respond, "has_login");
                    failure = 1;
                } else {
                    sprintf(sql_sentence,
                            "update account set online = 1 where id = '%s' and passwd = '%s'",
                            recv_pack->id, recv_pack->passwd);      // 设置在线
                    mysql_query(&mysql, sql_sentence);


                    UserData *prev = userdataHead;

                    while (cur != NULL) {       // 查看链表里是否存在这个id
                        if (strcmp(cur->id, recv_pack->id) == 0) {
                            cur->client_sockfd = recv_pack->client_sockfd; // 更新client_sockfd
                            cur->server_sockfd = recv_pack->server_sockfd; // 更新server_sockfd
                            flag = 1;
                            break;
                        }
                        prev = cur;
                        cur = cur->next;
                    }


                    if (flag == 0) {   // 链表里不存在这个id, 新建这个节点连接到链表中

                        strcpy(tmp->id, recv_pack->id);                 // 保存id
                        tmp->client_sockfd = recv_pack->client_sockfd;  // 保存client_sockfd
                        tmp->server_sockfd = recv_pack->server_sockfd;  // 保存server_sockfd
                        tmp->friend_request_num = 0;                    // 把好友请求数量置为0

                        if (userdataHead == NULL) { // 这是链表中插入的第一个结点
                            userdataHead = tmp;
                            tmp->next = NULL;
                        } else {
                            prev->next = tmp; // 尾插入链表中
                            tmp->next = NULL;
                        }
                    }
                    strcpy(send_pack->respond, "success");
                }
            }

            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0); // 发送给原客户端响应

            if (failure == 1) {
                break;
            }

            if (flag == 1) {
                // 发送给原客户端用户数据, 用来判断在离线期间是否有请求
                Send(recv_pack->server_sockfd, cur, sizeof(UserData), 0);
            } else {
                Send(recv_pack->server_sockfd, tmp, sizeof(UserData), 0);
            }
            break;
        }
        case REGISTER: {
            char s[32];
            MD5Change(s, recv_pack->passwd);

            char sql_sentence[500];

            sprintf(sql_sentence,
                    "insert into account(passwd, user_name, security_question, security_answer, online) values ('%s', '%s', '%s', '%s', %d)",
                    recv_pack->passwd, recv_pack->name, recv_pack->security_question, recv_pack->security_answer,
                    recv_pack->online);
            mysql_query(&mysql, sql_sentence); // 插入账号数据

            sprintf(sql_sentence,
                    "select * from account where passwd = '%s' and user_name = '%s' and security_question = '%s' and security_answer = '%s'",
                    recv_pack->passwd, recv_pack->name, recv_pack->security_question, recv_pack->security_answer);

            mysql_query(&mysql, sql_sentence); // 找到这个账号对应的id发给客户端
            result = mysql_store_result(&mysql); // 拿到结果集
            row = mysql_fetch_row(result); // 拿到那一行数据

//            printf("lalalalala\n");

//            char id[10];
//            sprintf(id, "%d", row[0]);
            strcpy(send_pack->id, row[0]); // row[0] 是第一列数据即id

            strcpy(send_pack->respond, "success");
            send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
            break;
        }
        case FIND_MY_PASSWD: {
            char sql_sentence[500];

            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
            } else {
                row = mysql_fetch_row(result); // 拿到那一行数据
                strcpy(send_pack->passwd, row[1]);
                strcpy(send_pack->security_question, row[3]);
                strcpy(send_pack->security_answer, row[4]);
                strcpy(send_pack->respond, "success");
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
            break;
        }
        case CHANGE_PASSWORD: {
            char sql_sentence[500];
            if (recv_pack->flag == 0) {
                sprintf(sql_sentence,
                        "select * from account where id = '%s' and passwd = '%s'", recv_pack->id, recv_pack->passwd);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                    strcpy(send_pack->respond, "fail");
                } else {
                    row = mysql_fetch_row(result); // 拿到那一行数据
                    strcpy(send_pack->security_question, row[3]);
                    strcpy(send_pack->security_answer, row[4]);
                    strcpy(send_pack->respond, "success");
                }
                Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
            } else if (recv_pack->flag == 1) {
                sprintf(sql_sentence,
                        "update account set passwd = '%s' where id = '%s' and passwd = '%s'",
                        recv_pack->client_msg, recv_pack->id, recv_pack->passwd);
                mysql_query(&mysql, sql_sentence);
            }
            break;
        }
        case EXIT: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
            } else {
                sprintf(sql_sentence,
                        "update account set online = 0 where id = '%s'", recv_pack->id);
                mysql_query(&mysql, sql_sentence); // 设置下线
                strcpy(send_pack->respond, "success");
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);

            Close(recv_pack->server_sockfd);
            printf("客户端与服务器断开连接\n");
            break;
        }
        case ADD_FRIEND: {

            // 知道原来客户端的client_sockfd server_sockfd
            // 在recv_pack->client_sockfd
            // 和 recv_pack->server_sockfd 中
            // 可以通过遍历链表 比对client_sockfd 找到这个客户端的id, 然后通过id查询数据库找到name

            // 知道好友客户端的账号( recv_pack->id ), client_sockfd server_sockfd
            // 是在链表中通过遍历链表比对id找到的
            // 可以通过id查询数据库知道好友的name

            char sql_sentence[500];

            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { //// 一行都没有, 该好友账户不存在
                strcpy(send_pack->respond, "fail");
            } else {
                UserData *newhead = userdataHead;
                while (newhead != NULL) {
                    if (newhead->client_sockfd == recv_pack->client_sockfd) {
                        ///// 判断是否是自己加自己好友 即好友id=自己id
                        if (strcmp(newhead->id, recv_pack->id) == 0) {
                            strcpy(send_pack->respond, "fail");
                            goto fail;
                        }

                        strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                        sprintf(sql_sentence,
                                "select * from account where id = '%s'", send_pack->id);
                        mysql_query(&mysql, sql_sentence);
                        result = mysql_store_result(&mysql); // 拿到结果集
                        row = mysql_fetch_row(result); // 拿到那一行数据
                        strcpy(send_pack->name, row[2]);    // 保存原客户端name
                        break;
                    }
                    newhead = newhead->next;
                }

                //// 判断是否已经是好友
                sprintf(sql_sentence,
                        "select * from friend where user_id = '%s' and friend_id = '%s'",
                        recv_pack->id, send_pack->id);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                    sprintf(sql_sentence,
                            "select * from friend where user_id = '%s' and friend_id = '%s'",
                            send_pack->id, recv_pack->id);
                    mysql_query(&mysql, sql_sentence);
                    result = mysql_store_result(&mysql); // 拿到结果集
                    rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                    if (rowcount == 0) {
                        ////////////////// 他们原来不是好友
                        sprintf(sql_sentence,
                                "select * from account where id = '%s'", recv_pack->id);
                        mysql_query(&mysql, sql_sentence);
                        result = mysql_store_result(&mysql); // 拿到结果集
                        row = mysql_fetch_row(result); // 拿到那一行数据
                        char online[5];
                        strcpy(online, row[5]);
                        if (strcmp(online, "1") == 0) { ////// 该好友在线, 找到好友客户端的sockfd,直接发送过去
                            UserData *head = userdataHead;
                            while (head != NULL) {
                                if (strcmp(head->id, recv_pack->id) == 0) {
                                    send_pack->client_sockfd = head->client_sockfd; // 保存好友客户端的sockfd
                                    send_pack->server_sockfd = head->server_sockfd; // 保存好友客户端对应的服务器sockfd
                                    break;
                                }
                                head = head->next;
                            }

                            strcpy(send_pack->server_msg, recv_pack->id);   // 保存好友客户端的账号

                            send_pack->type = ADD_FRIEND; // 设置send_pack的type
                            Send(send_pack->server_sockfd, send_pack, sizeof(Pack), 0);  // 发送给另一个客户端
                        } else { ///// 该好友不在线, 找到他对应的用户数据链表, 然后把friend_request_num++
                            UserData *head = userdataHead;
                            UserData *prev = userdataHead;
                            int flag = 0;
                            while (head != NULL) {
                                if (strcmp(head->id, recv_pack->id) == 0) {
                                    head->friend_request_num++; // friend_request_num++
                                    strcpy(head->friend_id[head->friend_request_num - 1], send_pack->id); // 保存原客户端id
                                    flag = 1;
                                    break;
                                }
                                prev = head;
                                head = head->next;
                            }

                            UserData *tmp = (UserData *) malloc(sizeof(UserData));

                            if (flag == 0) {         //////////// 虽然注册了账号, 但是没有登陆过, 只会进入1次
                                strcpy(tmp->id, recv_pack->id);                 // 保存好友客户端id
                                tmp->friend_request_num = 1;                    // 把好友请求数量置为1
                                strcpy(tmp->friend_id[0], send_pack->id);   // 保存原客户端id
                                if (userdataHead == NULL) { // 这是插入的第一个节点
                                    userdataHead = tmp;
                                    tmp->next = NULL;
                                } else {
                                    prev->next = tmp; // 尾插入链表中
                                    tmp->next = NULL;
                                }
                            }
                            char *string = (char *) malloc(sizeof(char) * 2000);

                            sprintf(string, "在你离线期间, 账号为%s, 昵称为%s的用户想添加你为好友", send_pack->id, send_pack->name);
                            if (flag == 1) {
                                strcpy(head->server_msg[head->friend_request_num - 1], string);
                            } else {
                                strcpy(tmp->server_msg[tmp->friend_request_num - 1], string);
                            }
                        }
                        strcpy(send_pack->respond, "success");
                    } else {
                        // 他们原来是好友
                        strcpy(send_pack->respond, "fail");
                    }
                } else {
                    // 他们原来是好友
                    strcpy(send_pack->respond, "fail");
                }
            }
            fail:
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0); // 发给原客户端响应
            break;
        }
        case HANDLE_ADD_FRIEND_REQUESTS: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "insert into friend value('%s', '%s', 1)",
                    recv_pack->client_msg, recv_pack->id);
            mysql_query(&mysql, sql_sentence);
        }
        case DEL_FRIEND: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
            } else {
                UserData *newhead = userdataHead;
                while (newhead != NULL) {
                    if (newhead->client_sockfd == recv_pack->client_sockfd) {
                        strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                        break;
                    }
                    newhead = newhead->next;
                }
                ///// 判断她们是否是好友
                sprintf(sql_sentence,
                        "select * from friend where user_id = '%s' and friend = '%s' union select * from friend where user_id = '%s' and friend = '%s'",
                        recv_pack->id, send_pack->id, send_pack->id, recv_pack->id);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { //////她们原来不是好友
                    strcpy(send_pack->respond, "fail");
                } else {  ////她们原来是好友, 要删除掉记录
                    sprintf(sql_sentence,
                            "delete from friend where user_id = '%s' and friend = '%s' union select * from friend where user_id = '%s' and friend = '%s'",
                            recv_pack->id, send_pack->id, send_pack->id, recv_pack->id);
                    mysql_query(&mysql, sql_sentence);
                    strcpy(send_pack->respond, "success");
                }
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
            break;
        }
        case VIEW_FRIEND_LIST: {
            char sql_sentence[500];
            UserData *newhead = userdataHead;
            while (newhead != NULL) {
                if (newhead->client_sockfd == recv_pack->client_sockfd) {
                    strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                    break;
                }
                newhead = newhead->next;
            }

            sprintf(sql_sentence,
                    "select * from friend where user_id = '%s' union select * from friend where friend_id = '%s'",
                    send_pack->id, send_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { //////没有记录, 没有好友
                strcpy(send_pack->respond, "fail");
                Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
            } else {
                int i = 0, j = 0;
                Box *box = (Box *) malloc(sizeof(Box));
                memset(box, 0, sizeof(Box));
                while (row = mysql_fetch_row(result)) {  // 拿到那一行数据
                    if (strcmp(row[0], send_pack->id) == 0) {
                        strcpy(box->id[i++], row[1]);
                        sprintf(sql_sentence,
                                "select * from account where id = '%s'", row[1]);
                        mysql_query(&mysql, sql_sentence);
                        result = mysql_store_result(&mysql); // 拿到结果集
                        box->online[j++] = atoi(row[5]);
                    } else {
                        strcpy(box->id[i++], row[0]);
                        sprintf(sql_sentence,
                                "select * from account where id = '%s'", row[1]);
                        mysql_query(&mysql, sql_sentence);
                        result = mysql_store_result(&mysql); // 拿到结果集
                        box->online[j++] = atoi(row[5]);
                    }
                }
                box->num = i;       // 有多少个好友
                strcpy(send_pack->respond, "success");
                Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
                Send(recv_pack->server_sockfd, box, sizeof(Box), 0);
            }
            break;
        }
        case ADD_BLACKLIST: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
            } else {     //// 这个账号存在, 判断是否是好友
                UserData *newhead = userdataHead;
                while (newhead != NULL) {
                    if (newhead->client_sockfd == recv_pack->client_sockfd) {
                        strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                        break;
                    }
                    newhead = newhead->next;
                }

                sprintf(sql_sentence,
                        "select * from friend where user_id = '%s' and friend_id = '%s' union select * from friend where user_id = '%s' and friend_id = '%s'",
                        send_pack->id, recv_pack->id, recv_pack->id, send_pack->id);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { //////没有记录, 不是好友
                    strcpy(send_pack->respond, "fail");
                } else {  /// 是好友
                    sprintf(sql_sentence,
                            "update friend set relationship = 2 where user_id = '%s' and friend_id = '%s' union select * from friend where user_id = '%s' and friend_id = '%s'",
                            send_pack->id, recv_pack->id, recv_pack->id, send_pack->id);
                    mysql_query(&mysql, sql_sentence);  // 设置黑名单
                    strcpy(send_pack->respond, "success");
                }
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
        }
        case DEL_BLACKLIST: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
            } else {     //// 这个账号存在, 判断是否是好友
                UserData *newhead = userdataHead;
                while (newhead != NULL) {
                    if (newhead->client_sockfd == recv_pack->client_sockfd) {
                        strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                        break;
                    }
                    newhead = newhead->next;
                }

                sprintf(sql_sentence,
                        "select * from friend where user_id = '%s' and friend_id = '%s' union select * from friend where user_id = '%s' and friend_id = '%s'",
                        send_pack->id, recv_pack->id, recv_pack->id, send_pack->id);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { //////没有记录, 不是好友
                    strcpy(send_pack->respond, "fail");
                } else {  /// 是好友
                    sprintf(sql_sentence,
                            "update friend set relationship = 1 where user_id = '%s' and friend_id = '%s' union select * from friend where user_id = '%s' and friend_id = '%s'",
                            send_pack->id, recv_pack->id, recv_pack->id, send_pack->id);
                    mysql_query(&mysql, sql_sentence);  // 删除黑名单关系
                    strcpy(send_pack->respond, "success");
                }
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
        }
        case CHAT_WITH_FRIEND: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { // 一行都没有, 这个账户数据不存在
                strcpy(send_pack->respond, "fail");
            } else {     //// 这个账号存在, 判断是否是好友
                UserData *newhead = userdataHead;
                while (newhead != NULL) {
                    if (newhead->client_sockfd == recv_pack->client_sockfd) {
                        strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                        break;
                    }
                    newhead = newhead->next;
                }

                sprintf(sql_sentence,
                        "select * from friend where user_id = '%s' and friend_id = '%s' union select * from friend where user_id = '%s' and friend_id = '%s'",
                        send_pack->id, recv_pack->id, recv_pack->id, send_pack->id);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { //////没有记录, 不是好友
                    strcpy(send_pack->respond, "fail");
                } else {  /// 是好友
                    if (strcmp(row[2], "2") == 0) {   /// 被拉黑了
                        strcpy(send_pack->respond, "fail");
                    } else {  /// 没有被拉黑
                        strcpy(send_pack->respond, "success");
                    }
                }
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0);
        }
        case REAL_CHAT: {
            char sql_sentence[500];

            UserData *newhead = userdataHead;
            while (newhead != NULL) {
                if (newhead->client_sockfd == recv_pack->client_sockfd) {

                    strcpy(send_pack->id, newhead->id);     /// 保存原客户端的id
                    sprintf(sql_sentence,
                            "select * from account where id = '%s'", send_pack->id);
                    mysql_query(&mysql, sql_sentence);
                    result = mysql_store_result(&mysql); // 拿到结果集
                    row = mysql_fetch_row(result); // 拿到那一行数据
                    strcpy(send_pack->name, row[2]);    /// 保存原客户端name
                    break;
                }
                newhead = newhead->next;
            }

            sprintf(sql_sentence,
                    "select * from account where id = '%s'", recv_pack->id);
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            row = mysql_fetch_row(result); // 拿到那一行数据
            char online[5];
            strcpy(online, row[5]);
            if (strcmp(online, "1") == 0) { ////// 该好友在线, 找到好友客户端的sockfd,直接发送过去
                UserData *head = userdataHead;
                while (head != NULL) {
                    if (strcmp(head->id, recv_pack->id) == 0) {
                        send_pack->client_sockfd = head->client_sockfd; // 保存好友客户端的sockfd
                        send_pack->server_sockfd = head->server_sockfd; // 保存好友客户端对应的服务器sockfd
                        break;
                    }
                    head = head->next;
                }

                strcpy(send_pack->server_msg, recv_pack->id);   // 保存好友客户端的账号
                strcpy(send_pack->message, recv_pack->message); // 保存消息内容
                send_pack->type = REAL_CHAT; // 设置send_pack的type
                Send(send_pack->server_sockfd, send_pack, sizeof(Pack), 0);  // 发送给另一个客户端
            } else { ///// 该好友不在线, 找到他对应的用户数据链表, friend_chat_num++
                UserData *head = userdataHead;
                UserData *prev = userdataHead;
                int flag = 0;
                while (head != NULL) {
                    if (strcmp(head->id, recv_pack->id) == 0) {
                        head->friend_chat_num++; // friend_chat_num++
                        strcpy(head->friend_id[head->friend_chat_num - 1], send_pack->id); // 保存原客户端id
                        flag = 1;
                        break;
                    }
                    prev = head;
                    head = head->next;
                }

                UserData *tmp = (UserData *) malloc(sizeof(UserData));

                if (flag == 0) {         //////////// 虽然注册了账号, 但是没有登陆过, 只会进入1次
                    strcpy(tmp->id, recv_pack->id);                 // 保存好友客户端id
                    tmp->friend_chat_num = 1;                    // 把好友请求数量置为1
                    strcpy(tmp->friend_id[0], send_pack->id);   // 保存原客户端id
                    if (userdataHead == NULL) { // 这是插入的第一个节点
                        userdataHead = tmp;
                        tmp->next = NULL;
                    } else {
                        prev->next = tmp; // 尾插入链表中
                        tmp->next = NULL;
                    }
                }
                char *string = (char *) malloc(sizeof(char) * 2000);

                sprintf(string, "在你离线期间, 账号为%s, 昵称为%s的用户私聊你, 内容为:%s", send_pack->id, send_pack->name,
                        recv_pack->message);
                if (flag == 1) {
                    strcpy(head->server_msg[head->friend_chat_num - 1], string);
                } else {
                    strcpy(tmp->server_msg[tmp->friend_chat_num - 1], string);
                }
            }
            break;
        }
//        case VIEW_FRIEND_CHAT_MSG: {
//
//        }
        case CREATE_GROUP: {
            char sql_sentence[500];
            UserData *newhead = userdataHead;
            while (newhead != NULL) {
                if (newhead->client_sockfd == recv_pack->client_sockfd) {
                    strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                    sprintf(sql_sentence,
                            "select * from account where id = '%s'", send_pack->id);
                    mysql_query(&mysql, sql_sentence);
                    result = mysql_store_result(&mysql); // 拿到结果集
                    row = mysql_fetch_row(result); // 拿到那一行数据
                    strcpy(send_pack->name, row[2]);    // 保存原客户端name
                    break;
                }
                newhead = newhead->next;
            }

            sprintf(sql_sentence,
                    "insert into group_outline(group_name, group_mem_num) values(%s, 1)", recv_pack->group_name);
            mysql_query(&mysql, sql_sentence);
            result2 = mysql_store_result(&mysql); // 拿到结果集
            row2 = mysql_fetch_row(result); // 拿到那一行数据
            char group_id[20];
            strcpy(group_id, row2[0]);

            sprintf(sql_sentence,
                    "insert into group_member(id, name, group_id, group_name, 1) values(%s, %s, %d, %s)",
                    send_pack->id, send_pack->name, atoi(group_id), recv_pack->group_name);
            mysql_query(&mysql, sql_sentence);
            strcpy(send_pack->respond, "success");
            break;
        }
        case EXIT_GROUP: {
            char sql_sentence[500];
            sprintf(sql_sentence,
                    "select * from group_outline where group_id = '%d'", atoi(recv_pack->group_id));
            mysql_query(&mysql, sql_sentence);
            result = mysql_store_result(&mysql); // 拿到结果集
            rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
            if (rowcount == 0) { //// 一行都没有, 该群不存在
                strcpy(send_pack->respond, "fail");
            } else {   /// 该群存在, 判断是否为群主
                UserData *newhead = userdataHead;
                while (newhead != NULL) {
                    if (newhead->client_sockfd == recv_pack->client_sockfd) {
                        strcpy(send_pack->id, newhead->id);     // 保存原客户端的id
                        sprintf(sql_sentence,
                                "select * from account where id = '%s'", send_pack->id);
                        mysql_query(&mysql, sql_sentence);
                        result = mysql_store_result(&mysql); // 拿到结果集
                        row = mysql_fetch_row(result); // 拿到那一行数据
                        strcpy(send_pack->name, row[2]);    // 保存原客户端name
                        break;
                    }
                    newhead = newhead->next;
                }

                sprintf(sql_sentence,
                        "select * from group_member where id = '%s'", send_pack->id);
                mysql_query(&mysql, sql_sentence);
                result = mysql_store_result(&mysql); // 拿到结果集
                rowcount = (int) mysql_num_rows(result);  //获得结果集有多少行
                if (rowcount == 0) { //// 一行都没有, 该群存在, 但该用户不在这个群
                    strcpy(send_pack->respond, "fail");
                } else {  /// 该用户在这个群
                    row = mysql_fetch_row(result); // 拿到那一行数据
                    if (strcmp(row[4], "1") == 0) {  /// 是群主
                        sprintf(sql_sentence,
                                "delete from group_outline where group_id = '%d'", atoi(send_pack->group_id));
                        mysql_query(&mysql, sql_sentence);

                        sprintf(sql_sentence,
                                "delete from group_member where group_id = '%d'", atoi(send_pack->group_id));
                        mysql_query(&mysql, sql_sentence);
                    } else {  /// 不是群主
                        sprintf(sql_sentence,
                                "delete from group_member where group_id = '%d' and id = '%s'",
                                atoi(send_pack->group_id), send_pack->id);
                        mysql_query(&mysql, sql_sentence);
                    }
                    strcpy(send_pack->respond, "success");
                }
            }
            Send(recv_pack->server_sockfd, send_pack, sizeof(Pack), 0); // 发送给原客户端响应
            break;
        }
        case VIEW_GROUP_LIST: {

        }
    }
}


int init_server() {
    printf("聊天室服务器开始启动...");
    int listen_fd;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("创建服务器socket失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 设置端口复用

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind 失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    printf("成功绑定\n");
    if (listen(listen_fd, 100) == -1) {
        perror("设置监听失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    printf("设置监听成功\n");
    printf("初始化服务器成功\n");
    return listen_fd;
}

void initLinkList() {
    userdataHead = NULL;
}