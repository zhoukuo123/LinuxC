#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <termios.h>
//#include <curses.h>
#include <ctype.h>
#include "func.h"
#include "menu.h"
#include "login.h"

// 未知
//#pragma warning( disable : 4996)

#define MAXLENGTH 1024
#define IP "localhost"
#define USERNAME "root"
#define PASSWORD "zhoukuo"

extern MYSQL mysql;
extern MYSQL_RES *result;
extern MYSQL_ROW row;
extern int rowcount;
extern char ch[2];

extern char login_id[MAXLENGTH];
extern char login_password[MAXLENGTH];
extern char login_name[MAXLENGTH];
extern char login_gender[MAXLENGTH];
extern char login_grade[MAXLENGTH];
extern char login_phone[MAXLENGTH];
extern char login_department[MAXLENGTH];
extern char query[MAXLENGTH];

extern int updateNum; // 更新用户信息的时候，用户选择的更新项
extern char update[MAXLENGTH]; // 更改后的内容

extern char checkBorrowed_ret[MAXLENGTH]; // 用于接收用户在看到借阅记录后的操作内容

extern char school_id[MAXLENGTH];//校园卡账号
extern char school_password[MAXLENGTH];//校园卡密码

//查询图书所用变量
extern char type1[MAXLENGTH];
extern char bookclass[MAXLENGTH];
extern char author[MAXLENGTH];
extern char bookname[MAXLENGTH];
extern char isbn[MAXLENGTH];
extern char bookid[MAXLENGTH];
extern char price[MAXLENGTH];
extern char publish[MAXLENGTH];
extern char location[MAXLENGTH];
extern char info[MAXLENGTH];
extern char writer[MAXLENGTH];
extern double priceUser;//用户输入的价格
extern int locationUser;//用户输入的书架位置

//针对查询图书进行下一次操作的变量
extern char book_id_next[MAXLENGTH];  //用户借书的

//统计
extern char type[MAXLENGTH];
extern char type2[MAXLENGTH];
extern char bookname1[MAXLENGTH];
extern char readername[MAXLENGTH];
extern char departmentid[MAXLENGTH];
extern char sexid[MAXLENGTH];
extern char borrowtimes[MAXLENGTH];
extern char grade[MAXLENGTH];

//查看罚金
extern char fines[MAXLENGTH];
extern char reader_id1[MAXLENGTH];
extern char reader_id2[MAXLENGTH];
extern char school_id1[MAXLENGTH];
extern char school_password1[MAXLENGTH];
extern char school_id2[MAXLENGTH];
extern char school_password2[MAXLENGTH];
extern char bookname2[MAXLENGTH];

#endif