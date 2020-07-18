#include "../common/main.h"

MYSQL mysql;
MYSQL_RES *result;
MYSQL_ROW row;
int rowcount;
char ch[2];
char login_id[MAXLENGTH];
char login_password[MAXLENGTH];
char login_name[MAXLENGTH];
char login_gender[MAXLENGTH];
char login_grade[MAXLENGTH];
char login_phone[MAXLENGTH];
char login_department[MAXLENGTH];
char query[MAXLENGTH];

int updateNum; // 更新用户信息的时候，用户选择的更新项
char update[MAXLENGTH]; // 更改后的内容

char checkBorrowed_ret[MAXLENGTH]; // 用于接收用户在看到借阅记录后的操作内容

char school_id[MAXLENGTH];//校园卡账号
char school_password[MAXLENGTH];//校园卡密码

//查询图书所用变量
char type1[MAXLENGTH];
char bookclass[MAXLENGTH];
char author[MAXLENGTH];
char bookname[MAXLENGTH];
char isbn[MAXLENGTH];
char bookid[MAXLENGTH];
char price[MAXLENGTH];
char publish[MAXLENGTH];
char location[MAXLENGTH];
char info[MAXLENGTH];
char writer[MAXLENGTH];
double priceUser;//用户输入的价格
int locationUser;//用户输入的书架位置

//针对查询图书进行下一次操作的变量
char book_id_next[MAXLENGTH];  //用户借书的

//统计
char type[MAXLENGTH];
char type2[MAXLENGTH];
char bookname1[MAXLENGTH];
char readername[MAXLENGTH];
char departmentid[MAXLENGTH];
char sexid[MAXLENGTH];
char borrowtimes[MAXLENGTH];
char grade[MAXLENGTH];

//查看罚金
char fines[MAXLENGTH];
char reader_id1[MAXLENGTH];
char reader_id2[MAXLENGTH];
char school_id1[MAXLENGTH];
char school_password1[MAXLENGTH];
char school_id2[MAXLENGTH];
char school_password2[MAXLENGTH];
char bookname2[MAXLENGTH];

int main() {

    //part0 设置窗口
    widget();

    // 初始化进度
    progressBar();

    //part1	初始化数据库
    mysql_init(&mysql);
    // progressBar(); //模拟初始化进度

    //第一部分：登陆+注册
    int identity = 0;

    while (1) {
        puts("\033[2J"); // 清屏操作
        loginMenu();
        char input = getchar();
        // usleep(500);
        if (input == '1') {
            puts("\033[2J"); // 清屏操作
            if (readerLogin() != 0) {
                // usleep(800)
                continue;
            }
            identity = 1;
            break;
        } else if (input == '2') {
            puts("\033[2J"); // 清屏操作
            if (adminLogin() != 0) {
                continue;
            }
            identity = 2;
            break;
        } else if (input == '3') {
            puts("\033[2J"); // 清屏操作
            if (registerAccount() == 0) {
                continue;
            }
        } else if (input == '4') {
            puts("\033[2J"); // 清屏操作
            printf("感谢您的使用!");
            exit(0);
        } else {
            continue;
        }
    }


    // 功能界面选择
    // 读者菜单部分
    if (identity == 1) {
        while (1) {
            puts("\033[2J"); // 清屏操作
            readerMenu();
            char input = getchar();
            // 借书
            if (input == '1') {
                puts("\033[2J"); // 清屏操作
                printf("\n\n\t\t\t将跳转到图书查询菜单，并选择下一步的操作\n\t\t\t");
                getchar();
                if (Checkbook(1) == 0) {
                    continue;
                } else {
                    borrowBooks(book_id_next, login_id);
                    continue;
                }
            } else if (input == '2') {  //查询图书信息并还书
                puts("\033[2J"); // 清屏操作
                if (checkBorrowed() == 0) {
                    continue;
                } else {
                    returnBooks(checkBorrowed_ret, login_id);
                    continue;
                }
            } else if (input == '3') { //查询个人信息并修改
                puts("\033[2J"); // 清屏操作
                if (checkInfo() == 1) {
                    getchar();
                    continue;
                } else {
                    updateInfo(updateNum, login_id);
                    continue;
                }
                continue;
            } else if (input == '4') { //申请续借
                puts("\033[2J"); // 清屏操作
                if (checkRenew() == 1) {
                    continue;
                } else {
                    renew(checkBorrowed_ret, login_id);
                    continue;
                }
            } else if (input == '5') { //交罚金
                puts("\033[2J"); // 清屏操作
                payDelayPenalty(login_id);
                continue;
            } else if (input == '0') { //退出
                puts("\033[2J"); // 清屏操作
                printf("感谢您的使用！");
                exit(0);
            } else {
                continue;
            }
        }
    } else { //管理员菜单
        while (1) {
            puts("\033[2J"); // 清屏操作
            adminMenu();
            char input = getchar();
            usleep(500);
            if (input == '1') {
                puts("\033[2J"); // 清屏操作
                SecPi();
                continue;
            } else if (input == '2') {
                puts("\033[2J"); // 清屏操作
                ModifyPi();
                continue;
            } else if (input == '3') {
                puts("\033[2J"); // 清屏操作
                Operatebook();
                continue;
            } else if (input == '4') {
                puts("\033[2J"); // 清屏操作
                borrowinfo();
                continue;
            } else if (input == '5') {
                puts("\033[2J"); // 清屏操作
                checkfines();
                continue;
            } else if (input == '6') {
                puts("\033[2J"); // 清屏操作
                countlist();
                continue;
            } else if (input == '0') {
                break;
            } else {
                continue;
            }
        }
    }

    return 0;
}
