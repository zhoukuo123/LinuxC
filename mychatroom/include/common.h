#ifndef MYCHAT_COMMON_H
#define MYCHAT_COMMON_H


#define PORT 8002
#define IP "localhost"
#define USERNAME "root"
#define PASSWORD "zhoukuo"

///////////////////// 账号管理 //////////////////////
#define LOGIN 0                     // 登陆
#define REGISTER 1                  // 注册
#define FIND_MY_PASSWD 2            // 找回密码
#define CHANGE_PASSWORD 30          // 修改密码
#define EXIT 3                      // 退出
#define NOTUSE 40                   // 用来使客户端接受线程往后执行时不进入任何case

///////////////////// 好友管理 //////////////////////
#define ADD_FRIEND 4                // 添加好友
#define DEL_FRIEND 5                // 删除好友
#define SEARCH_FRIEND 6             // 查询好友(去掉)
#define VIEW_FRIEND_LIST 6          // 查看好友列表
#define VIEW_FRIEND_CHAT_MSG 7      // 查看好友聊天记录
#define ADD_BLACKLIST 8             // 添加黑名单(屏蔽好友消息)
#define DEL_BLACKLIST 9            // 解除黑名单
#define CHAT_WITH_FRIEND 10         // 私聊前的检测
#define REAL_CHAT  48               // 真的私聊
#define VIEW_MESSAGE 11             // 查看消息
#define HANDLE_ADD_FRIEND_REQUESTS 23   // 处理添加好友请求
#define HANDLE_ADD_FRIEND_REQUESTS2 24  // 处理添加好友请求2



///////////////////// 群管理 ///////////////////////
#define CREATE_GROUP 12             // 创建群
#define EXIT_GROUP 13               // 退出群(如果在群中的身份为群主, 则解散群)
#define JOIN_GROUP 14               // 申请加群
#define VIEW_GROUP_LIST 15          // 查看群列表
#define VIEW_GROUP_MEMBER 16        // 查看群成员
#define VIEW_GROUP_CHAT_MSG 17      // 查看群聊天记录
#define SET_GROUP_MANAGER 18        // 设置群管理员(群主才有的权限)
#define KICK_PEOPLE 19              // 踢人(群主或管理员才有的权限)

///////////////////// 文件管理 //////////////////////
#define SEND_FILE 20 // 发送文件
#define RECV_FILE 21 // 接受文件

void view_group_list(int client_sockfd);

void exit_group(int client_sockfd);

void create_group(int client_sockfd);

void chat_with_friend(int client_sockfd);

void add_blacklist(int client_sockfd);

void del_blacklist(int client_sockfd);

void login(int client_sockfd, int *is_login);

void register1(int client_sockfd);

void find_my_passwd(int client_sockfd);

void change_password(int client_sockfd);

void exit_function(int client_sockfd);

int getch();

int init_server();

void *handle_thread(void *arg);

void add_friend(int client_sockfd);

void initLinkList();

void *response_thread(void *arg);

void view_messaeg(int client_sockfd);

void del_friend(int client_sockfd);

void view_friend_list(int client_sockfd);


/**
 * 用户数据链表
 */
typedef struct user_data {
    char id[10];
    char friend_id[50][10];  // 最多50个好友请求
    char passwd[30];
    char name[20];
    char security_question[30];
    char security_answer[30];
    int online; // 0 不在线, 1 在线
    int client_sockfd;
    int server_sockfd;

    int friend_request_num; // 好友请求数量
    int friend_chat_num;    // 好友私聊数量
    int group_request_num;  // 加群请求数量

    char client_msg[50];    // 给客户端的一个空间

    char respond[20];       // 服务器对客户端的响应
    char server_msg[50][500];// 给服务器的一个空间(最多50个好友请求)
    struct user_data *next;
} UserData;

/**
 * 包
 */
typedef struct pack {
    int type;

    char id[10];
    char group_id[10];
    char passwd[30];
    char name[20];
    char group_name[20];
    char security_question[30];
    char security_answer[30];
    int online; // 0 不在线, 1 在线
    int client_sockfd;
    int server_sockfd;

    int flag;               // 客户端给服务器的一个信息标志位(接受 1 or 拒绝 0)
    char client_msg[50];    // 给客户端的一个空间
    char message[50];       // 私聊的消息内容

    char respond[20];       // 服务器对客户端的响应
    char server_msg[50];    // 给服务器的一个空间
} Pack;


/**
 * 离线传输的处理: 消息盒子
 * 离线消息, 上线及时发送
 * 离线文件, 上线及时提醒有文件传送
 */
typedef struct msg_box {
    char id[50][10];
    char passwd[30];
    char name[50][20];
    char security_question[30];
    char security_answer[30];
    int online[50]; // 0 不在线, 1 在线
    int client_sockfd;
    int server_sockfd;

    int friend_request_num; // 好友请求数量
    int friend_chat_num;    // 好友私聊数量
    int group_request_num;  // 加群请求数量

    char message[50][50];   // 私聊消息内容
    int num;
    char client_msg[50][50];    // 给客户端的一个空间

    char respond[20];       // 服务器对客户端的响应
    char server_msg[50][50];// 给服务器的一个空间(最多50个好友请求)
//    struct user_data *next;
} Box;


/**
 * 文件
 */
typedef struct file {
    int size;
    char file_name[20];
    char content[BUFSIZ];
} file;


#endif //MYCHAT_COMMON_H
