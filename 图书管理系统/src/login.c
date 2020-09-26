#include "../include/login.h"

int getch(void);

//窗口配置
void widget() {
// 修改窗口大小

// 设置窗口标题
}

//初始化进度
void progressBar() {
    puts("\033[2J"); // 清屏操作
    printf("加载中, 请稍等...\n");
    int i = 0;
    char bar[102] = {0};
    char *lab = "-\\|/"; // 转义
    while (i <= 100) {
        printf("[%-101s][%d%%][%c]\r", bar, i, lab[i % 4]);
        fflush(stdout);
        if (i <= 10) {
            usleep(60000);
        } else if (i <= 60) {
            usleep(15000);
        } else {
            usleep(30000);
        }
        bar[i++] = '#';
        bar[i] = '\0';
    }
    printf("\n加载成功！\n");
    usleep(110000);
    puts("\033[2J"); // 清屏操作
}

/**
 * 注册功能
 * @return 返回0为注册成功, 返回1为注册失败
 */
int registerAccount() {
    mysql_init(&mysql);
    // 首先连接数据库，连接失败打印信息，断开连接并退出程序

    if (mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0) == NULL) {
        printf("数据库有问题, 导致连接不上了\n");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'utf8'"); // 设置输出编码
        mysql_query(&mysql, "set autocommit=0;"); // 禁用自动提交事务
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        //接着看是否能连接到借阅人的表
        // mysql_query 函数调用成功返回0
        if (mysql_query(&mysql, "select * from reader")) {
            printf("保存信息的表格连不上了\n");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else { // 现在，可以进入注册功能了
            //打印提示信息
            printf("\t════════════════════════════\n");
            printf("\t          借阅人注册          \n");
            printf("\t════════════════════════════\n");

            result = mysql_store_result(&mysql); // 获得结果集
            rowcount = (int) mysql_num_rows(result); //获得结果集有多少行

            //首先对唯一标识：用户名进行判断【长度+唯一两部分】
            while (1) {
                printf("\t请输入用户名【15位以内】: ");
                scanf("%s", login_id);
                //长度太长更换
                if (strlen(login_id) > 15) {
                    printf("用户名过长.\n");
                    continue;
                }
                //之后判断如果没有记录，就不用查重了，直接记录并进行下一步
                if (rowcount == 0) break;
                    //之后进行查重
                else {
                    //拿出每一行
                    int flag = 1;
                    while (row = mysql_fetch_row(result)) {
                        if (!strcmp(row[0], login_id)) {
                            printf("\t用户名已被注册!\n");
                            flag = 0;
                            mysql_free_result(result);

                            mysql_query(&mysql, "select * from reader"); //重新获取结果
                            result = mysql_store_result(&mysql); // 获得结果集
                            rowcount = (int) mysql_num_rows(result); // 获得结果集有多少行

                            break;
                        }
                    }
                    if (flag == 1) {
                        break;
                    } else {
                        continue;
                    }
                }
            }

            //接着是密码，长度太长更换
            while (1) {
                printf("\n\t请输入密码【15位以内】: ");
                scanf("%s", login_password);
                if (strlen(login_password) > 15) {
                    printf("密码过长!\n");
                    continue;
                } else {
                    break;
                }
            }

            //接着是名字，长度太长更换
            while (1) {
                printf("\n\t请输入名字【15位以内】: ");
                scanf("%s", login_name);
                if (strlen(login_name) > 15) {
                    printf("名字过长!\n");
                    continue;
                } else {
                    break;
                }
            }

            //性别只能是1和2
            while (1) {
                printf("\n\t请输入性别【男1 女2】: ");
                scanf("%s", login_gender);
                if (strcmp(login_gender, "1") != 0 && strcmp(login_gender, "2") != 0) {
                    printf("输入有误!\n");
                    continue;
                } else {
                    break;
                }
            }

            //身份只能是1-7
            while (1) {
                printf("\n\t请输入身份【大一 1，大二 2，大三 3，大四 4，研究生 5，博士生 6，教师 7】: ");
                scanf("%s", login_grade);
                if (strcmp(login_grade, "1") != 0 && strcmp(login_grade, "2") != 0
                    && strcmp(login_grade, "3") != 0 && strcmp(login_grade, "4") != 0
                    && strcmp(login_grade, "5") != 0 && strcmp(login_grade, "6") != 0
                    && strcmp(login_grade, "7") != 0) {
                    printf("输入有误!\n");
                    continue;
                } else {
                    break;
                }
            }

            //手机号 也不能超过15位
            while (1) {
                printf("\n\t请输入手机号: ");
                scanf("%s", login_phone);
                if (strlen(login_phone) > 15) {
                    printf("输入有误!\n");
                    continue;
                } else {
                    break;
                }
            }

            //学院只能是1-4
            char infoLint[100] = "请输入学院 [计算机学院 1，理学院 2，文法学院 3，生命科学学院 4]";
            while (1) {
                printf("\n\t%s", infoLint);
                scanf("%s", login_department);
                if (strcmp(login_department, "1") != 0 && strcmp(login_department, "2") != 0
                    && strcmp(login_department, "3") != 0 && strcmp(login_department, "4") != 0) {
                    printf("输入有误!\n");
                    continue;
                } else {
                    break;
                }
            }
        }

        //拼接语句
        memset(query, 0, sizeof(char) * MAXLENGTH);
        strcat(query, "insert into reader values('");
        strcat(query, login_id);
        strcat(query, "','");
        strcat(query, login_password);
        strcat(query, "','");
        strcat(query, login_name);
        strcat(query, "','");
        strcat(query, login_gender);
        strcat(query, "','");
        strcat(query, login_grade);
        strcat(query, "','");
        strcat(query, login_phone);
        strcat(query, "','");
        strcat(query, login_department);
        strcat(query, "',");
        strcat(query, "0,20,2,0)");

        // printf("%s", query);

        if (mysql_query(&mysql, query) != 0) {//将dest插入到数据库中(db_books)
            puts("\033[2J"); // 清屏操作
            fprintf(stderr, "数据库查询失败\n%s\n按任意键结束", mysql_error(&mysql));
            getchar();
            mysql_free_result(result);
            mysql_query(&mysql, "rollback");
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\n\n\t【注册成功!】\n");
            usleep(1500);
            mysql_free_result(result); //释放结果集
            mysql_query(&mysql, "commit");
            mysql_close(&mysql); //释放连接
            return 0;
        }
    }
}

/**
 * 借阅人登陆
 * @return 返回0为注册成功 返回1为密码错误 出现其他错误暂停后退出程序
 */
int readerLogin() {
    //连接数据库
    mysql_init(&mysql);
    // 返回NULL 为连接失败
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("数据库有问题, 导致连接不上了\n");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //查询表是否存在
        mysql_query(&mysql, "set names 'utf8'");
        mysql_query(&mysql, "set autocommit=0");

        if (mysql_query(&mysql, "select * from reader")) {
            printf("保存信息的表格连不上了\n");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            ReaderLoginMenu();

            result = mysql_store_result(&mysql);
            rowcount = (int) mysql_num_rows(result);

            if (rowcount == 0) {
                printf("\n\t\t\t【借阅人数据为空, 请先注册】\n按任意键回到上一级菜单");
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback");
                mysql_close(&mysql);
                return 1;
            } else {
                int have_id = 0, have_password = 0;
                printf("\n\t\t\t请输入您的用户名：");
                scanf("%s", login_id);

                printf("\n\t\t\t请输入您的密码：");
                char c = '\0';
                int i = 0;
                setbuf(stdin, NULL); // 清空缓冲区
                while ((c = getch()) != '\r') { // '\r'是回车符'\n'是换行符 注意
                    // int isprint(int c) 在头文件ctype.h中
                    // 检查所传的字符是否是可打印的。可打印字符是非控制字符的字符。
                    // 如果 c 是一个可打印的字符, 返回非0值, 否则返回0
                    if (c == '\b' && i > 0) {  //删除功能
                        i--;
                        printf("\b \b");
                    }

                    if (i < MAXLENGTH && isprint(c)) {
                        login_password[i++] = c;
                        putchar('*');
                    }
//                    else if (i > 0 && c == '\b') {
//                        --i;
//                        putchar('\b');
//                        putchar(' ');
//                        putchar('\b');
//                    }

                    putchar('\n');
                login_password[i] = '\0';

                while (row = mysql_fetch_row(result)) {
                    if (!strcmp(row[0], login_id)) {
                        have_id = 1;
                        if (!strcmp(row[1], login_password)) {
                            have_password = 1;
                            break;
                        }
                        break;
                    }
                }

                if (have_id == 0) {
                    printf("\n\t\t\t【用户名不存在!】\n\n\n\t\t\t");
                    getchar();
                    mysql_free_result(result);
                    mysql_query(&mysql, "rollback");
                    mysql_close(&mysql);
                    return 1;
                } else if (have_password == 0) {
                    printf("\n\t\t\t【密码不对!】\n\n\n\t\t\t");
                    getchar();
                    mysql_free_result(result);
                    mysql_query(&mysql, "rollback");
                    mysql_close(&mysql);
                    return 1;
                } else {
                    printf("\n\n\t\t\t【尊贵的读者，欢迎欢迎！】");
                    usleep(800);
                    mysql_free_result(result);
                    mysql_query(&mysql, "commit;");
                    mysql_close(&mysql);
                    return 0;
                }
            }
        }
    }
}

/**
 * 管理员登陆
 * @return 返回0为注册成功 返回1为密码错误 出现其他错误暂停后退出程序
 */
int adminLogin() {
    //连接数据库
    mysql_init(&mysql);
    if (mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0) == NULL) {
        printf("数据库有问题, 导致连接不上了\n");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //查询表是否存在
        mysql_query(&mysql, "set names 'utf8'");
        if (mysql_query(&mysql, "select * from admin")) {
            printf("保存信息的表格连不上了\n");
            getchar();
            mysql_close(&mysql);
            return 1;
        } else {
            AdminLoginMenu();
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            if (rowcount == 0) {
                printf("\n\t\t\t【管理员账户数据为空");
                getchar();
                mysql_free_result(result);
                mysql_close(&mysql);
                return 1;
            } else {
                int have_id = 0, have_password = 0;
                printf("\n\t\t\t请输入您的用户名：");
                scanf("%s", login_id);

                printf("\n\t\t\t请输入您的密码：");
                char c = '\0';
                int i = 0;
//                initscr(); // getch()使用前的初始化
                setbuf(stdin, NULL);
                while ((c = getch()) != '\r') { // '\r'是回车符'\n'是换行符
                    // int isprint(int c) 在头文件ctype.h中
                    // 检查所传的字符是否是可打印的。可打印字符是非控制字符的字符。
                    // 如果 c 是一个可打印的字符, 返回非0值, 否则返回0
                    if (i < MAXLENGTH && isprint(c)) {
                        login_password[i++] = c;
                        putchar('*');
                    } else if (i > 0 && c == '\b') {
                        --i;
                        putchar('\b');
                        putchar(' ');
                        putchar('\b');
                    }
                }
//                endwin(); // getch()使用后的注销
                putchar('\n');
                login_password[i] = '\0';

                while (row = mysql_fetch_row(result)) {
                    if (!strcmp(row[0], login_id)) {
                        have_id = 1;
                        if (!strcmp(row[1], login_password)) {
                            have_password = 1;
                            break;
                        }
                        break;
                    }
                }

                if (have_id == 0) {
                    printf("\n\t\t\t【用户名不存在!】\n\n\n\t\t\t");
                    getchar();
                    mysql_free_result(result);
                    mysql_close(&mysql);
                    return 1;
                } else if (have_password == 0) {
                    printf("\n\t\t\t【密码不对!】\n\n\n\t\t\t");
                    getchar();
                    mysql_free_result(result);
                    mysql_close(&mysql);
                    return 1;
                } else {
                    printf("\n\n\t\t\t【尊贵的管理员用户，欢迎欢迎！】");
                    usleep(800);
                    mysql_free_result(result);
                    mysql_close(&mysql);
                    return 0;
                }
            }
        }
    }
}











