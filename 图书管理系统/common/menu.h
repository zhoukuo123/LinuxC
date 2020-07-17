#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "main.h"

#define _CRT_SECURE_NO_WARNINGS

//菜单1：登陆+注册选择
void loginMenu();

//菜单2：借阅人菜单
void readerMenu();

//菜单3：管理员菜单
void adminMenu();

//菜单4：图书信息功能菜单
void bookinforMenu1();

// 菜单5：分类查询菜单
void bookinforMenu2();

//菜单6：统计菜单
void countMenu();

//菜单7：查询借书信息的方式
void bookinforMenu3();

//菜单8：管理员登录
void AdminLoginMenu();

//菜单9：读者登录
void ReaderLoginMenu();
//菜单10：查看罚金
void checkfinesMenu();

