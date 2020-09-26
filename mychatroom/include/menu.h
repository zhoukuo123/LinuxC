#ifndef LINUXC_MENU_H
#define LINUXC_MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <ctype.h>

// 菜单1：账号管理菜单(开始菜单)
void start_menu();

// 菜单2：二级菜单(选择 是进入好友管理 还是群管理 还是文件管理)
void secondary_menu();

// 菜单3：好友管理菜单
void friends_management_menu();

// 菜单4：群管理菜单
void group_management_menu();

// 菜单5：文件管理菜单
void file_management_menu();


#endif //LINUXC_MENU_H
