#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "main.h"

#define _CRT_SECURE_NO_WARNINGS

///////借阅者的函数功能部分///////
//借书功能（先查询，再借书）
int borrowBooks(char book_id[MAXLENGTH], char read_id[MAXLENGTH]);

//还书---查询借阅情况+还书
int checkBorrowed();
int returnBooks(char book_id[MAXLENGTH], char reader_id[MAXLENGTH]);

//查询个人信息
int checkInfo();
int updateInfo(int updateNum, char read_id[MAXLENGTH]);

//申请续借
int checkRenew();
int renew(char book_id[MAXLENGTH], char reader_id[MAXLENGTH]);

//交延期罚金
int payDelayPenalty(char reader_id[MAXLENGTH]);

/////////////管理员的函数功能部分//////////
//查询个人信息
int SecPi();

//修改个人信息
int ModifyPi();

//查询图书信息
int checkbook(int flag);




