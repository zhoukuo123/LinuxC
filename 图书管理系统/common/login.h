#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "main.h"

#define _CRT_SECURE_NO_WARNINGS

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

