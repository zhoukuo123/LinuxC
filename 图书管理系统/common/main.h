#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include "../common/func.h"
#include "../common/menu.h"
#include "../common/login.h"

// 未知
#define _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4996)

#define MAXLENGTH 1024
#define IP "localhost"
#define USERNAME "root"
#define PASSWORD "zhoukuo"

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

#define N 4



