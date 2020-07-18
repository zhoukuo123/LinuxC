#ifndef __FUNC_H__
#define __FUNC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include "main.h"

///////借阅者的函数功能部分///////
//借书功能（先查询，再借书）
int borrowBooks(char book_id[1024], char read_id[1024]);

//还书---查询借阅情况+还书
int checkBorrowed();

int returnBooks(char book_id[1024], char reader_id[1024]);

//查询个人信息
int checkInfo();

int updateInfo(int updateNum, char read_id[1024]);

//申请续借
int checkRenew();

int renew(char book_id[1024], char reader_id[1024]);

//交延期罚金
int payDelayPenalty(char reader_id[1024]);

/////////////管理员的函数功能部分//////////
//查询个人信息
int SecPi();

//修改个人信息
int ModifyPi();

//查询图书信息
int Checkbook(int flag);

//增添图书
int Addbook();

//修改图书
int Modifybook();

//删除图书
int Deletebook();

//操作图书信息
int Operatebook();

//查看借书信息
int borrowinfo();

//统计
int countlist();

//查询罚金列表
int checkfines();

#endif