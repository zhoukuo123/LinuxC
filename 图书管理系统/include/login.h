#ifndef __LOGIN_H__
#define __LOGIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
//#include <curses.h>
#include <ctype.h>
#include "main.h"

//窗口配置
void widget();

//初始化进度
void progressBar();

//注册功能
int registerAccount();

//借阅人登陆
int readerLogin();

//管理员登陆
int adminLogin();

#endif