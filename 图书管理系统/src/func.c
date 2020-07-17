#include "../common/func.h"

//////////////////////借阅者的函数功能部分///////////////////////
/**
 * 借书---查询图书信息+借书
 * @param book_id
 * @param reader_id
 * @return 返回0 借阅成功，返回1 表示有罚金现象或者所借图书超过上限
 */
int borrowBooks(char book_id[MAXLENGTH], char reader_id[MAXLENGTH]) {
    //首先连接数据库
    mysql_init(&mysql);
    if (mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0) == NULL) {
        printf("数据库有问题, 导致连接不上了\n");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        memset(query, 0, sizeof(char) * MAXLENGTH);
        strcat(query, "select * from book where book_id = '");
        strcat(query, book_id);
        strcat(query, "'");
        if (mysql_query(&mysql, query)) {
            if (mysql_query(&mysql, query)) {
                printf("保存信息的表格连不上了\n");
                getchar();
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            } else {
                result = mysql_store_result(&mysql);//获得结果集
                rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                row = mysql_fetch_row(result);
                if (rowcount == 0) {
                    printf("\n\t\t\t本书不存在！\n\t\t\t");
                    getchar();
                    mysql_free_result(result);    //释放结果集
                    mysql_query(&mysql, "rollback;");
                    mysql_close(&mysql);          //释放连接
                    return 1;
                } else if (strcmp(row[7], "0") != 0) {
                    printf("\n\t\t\t这本书图书馆里已经没有存货了！\n\t\t\t");
                    getchar();
                    mysql_free_result(result);    //释放结果集
                    mysql_query(&mysql, "rollback;");
                    mysql_close(&mysql);          //释放连接
                    return 1;
                }

                mysql_free_result(result);    //释放结果集
                mysql_query(&mysql, "select * from reader");
                result = mysql_store_result(&mysql);//获得结果集
                rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                //借书是在查询书之后，所以用户名和密码都存在，图书信息也是存在的，因而只存在有罚金和所借图书超过20本的现象

                while (row = mysql_fetch_row(result)) {
                    //找到了这个人
                    if (!strcmp(row[0], reader_id)) {
                        //有罚金返回到菜单界面
                        //printf("%d", atoi(row[10]));
                        if (atoi(row[10]) == 1) {
                            printf("\n\t\t\t处于罚款中, 不能借阅图书!\n\t\t\t");
                            getchar();
                            mysql_free_result(result);    //释放结果集
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);          //释放连接
                            return 1;
                        }
                    }

                    //如果所借已经=20本，返回菜单
                    //printf("%d", atoi(row[7]));
                    //getchar();
                    if (atoi(row[7]) >= 20) {
                        printf("\n\t\t\t所借图书超过20本, 不能再借!\n\t\t\t");
                        getchar();
                        mysql_free_result(result);    //释放结果集
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);          //释放连接
                        return 1;
                    }

                    //开始进行借书操作
                    //读者表
                    //所借+1
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "update reader set reader_have_borrowed=reader_have_borrowed+1 where reader_id = ");
                    strcat(query, "'");
                    strcat(query, reader_id);
                    strcat(query, "'");
                    mysql_query(&mysql, query);

                    //可借-1
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query,
                           "update reader set reader_left_borrowTimes=reader_left_borrowTimes-1 where reader_id = ");
                    strcat(query, "'");
                    strcat(query, reader_id);
                    strcat(query, "'");
                    mysql_query(&mysql, query);

                    //处理图书表，被借后状态变为1
                    mysql_free_result(result);
                    mysql_query(&mysql, "select * from book");//重新获取结果
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows((result));//获得结果集有多少行

                    while (row = mysql_fetch_row(result)) {
                        //找到了这本书
                        if (!strcmp(row[0], book_id)) {
                            memset(query, 0, sizeof(char) * MAXLENGTH);
                            strcat(query, "update book set borrowed=1 where book_id = ");
                            strcat(query, "'");
                            strcat(query, book_id);
                            strcat(query, "'");
                            mysql_query(&mysql, query);
                            break;
                        }
                    }

                    //最后添加借阅记录
                    mysql_free_result(result);
                    if (mysql_query(&mysql, "select * from borrowed")) {
                        printf("\n\t【保存借阅信息的表格连接失败\n请按任意键结束");
                        getchar();
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    }

                    result = mysql_store_result(&mysql);
                    rowcount = (int) mysql_num_rows(result);

                    //拼接语句
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "insert into borrowed values('");
                    strcat(query, book_id);
                    strcat(query, "','");
                    strcat(query, reader_id);
                    strcat(query, "',");
                    strcat(query, "now()");
                    strcat(query, ",");
                    strcat(query, "DATE_SUB(now(), INTERVAL -30 DAY),1"); // 注意: 设置归还时间
                    strcat(query, ")");

                    if (mysql_query(&mysql, query) != 0) { //将dest插入到数据库中(db_books)
                        system("clear");
                        printf("【保存借阅信息的数据库连接失败】\n%s\n请按任意键结束...", mysql_error(&mysql));
                        getchar();
                        mysql_free_result(result);
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    } else {
                        printf("\n\n\t\t\t【借书成功！！!】\n");
                        sleep(800);
                        mysql_free_result(result);    //释放结果集
                        mysql_query(&mysql, "commit;");
                        mysql_close(&mysql);          //释放连接
                        return 0;
                    }
                }
            }
        }
    }
}

//-------------------链表1
struct Node1 {
    //数据域
    char row0[MAXLENGTH];
    char row1[MAXLENGTH];
    char row2[MAXLENGTH];
    char row3[MAXLENGTH];
    char row4[MAXLENGTH];
    struct Node1 *next; //指针域(指向节点的指针）
};

struct Node1 *head1 = NULL;
struct Node1 *end1 = NULL;

void AddListTill1(char *row0, char *row1, char *row2, char *row3, char *row4) {
    //创建一个节点
    struct Node1 *temp = (struct Node1 *) malloc(sizeof(struct Node1));
    //节点数据进行赋值
    strcpy(temp->row0, row0);
    strcpy(temp->row1, row1);
    strcpy(temp->row2, row2);
    strcpy(temp->row3, row3);
    strcpy(temp->row4, row4);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head1) {
        head1 = temp;
    } else {
        end1->next = temp;
    }
    end1 = temp; // 尾结点应该始终指向最后一个
}

void ScanList1() {
    struct Node1 *temp = head1; //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("\t%s\t%-30s\t%-30s\t%-30s\t%s\n", temp->row0, temp->row1, temp->row2, temp->row3, temp->row4);
        temp = temp->next;
    }
}

void FreeList1() {
    //一个一个NULL
    struct Node1 *temp = head1;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node1 *pt = temp;
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                    //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head1 = NULL;
    end1 = NULL;
}

//还书---查询借阅情况+还书
int checkBorrowed() {
    //首先连接数据库
    mysql_init(&mysql);
    if (mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0) == NULL) {
        printf("数据库有问题, 导致连接不上了\n");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        if (mysql_query(&mysql, "select * from borrowed")) {
            printf("\n\t【保存借阅信息的表格连接失败\n请按任意键结束");
            getchar();
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\t════════════════════════════\n");
            printf("\t          查询借阅情况          \n");
            printf("\t════════════════════════════\n");

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            if (rowcount == 0) {
                printf("\n\t【看来你还没借书啊，先去借一本书再来吧】\n按任意键回到上一级菜单借本书！");
                //sleep(1000);
                getchar();
                mysql_free_result(result);
                mysql_close(&mysql);
                return 1;
            } else {
                mysql_free_result(result);

                memset(query, 0, sizeof(char) * MAXLENGTH);
                strcat(query, "select b.book_id, book_name, borrow_times, return_plan, IF(return_plan < NOW(), '超期有罚金', '未超期') penalty\
                    from borrowed bd\
inner join book b\
ON bd.`book_id` = b.`book_id`\
where valid = 1 and reader_id ='");
                strcat(query, login_id);
                strcat(query, "'");
                mysql_query(&mysql, query);

                result = mysql_store_result(&mysql);//获得结果集
                rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

                printf("\t══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n");
                printf("\t%s\t%-30s\t%-30s\t%-30s\t%s\n", "书号", "书名", "借书时间", "应还时间", "罚金");
                //printf("\t书号\t书名\t\t\t借书时间\t\t应还时间\t\t罚金\n");
                while (row = mysql_fetch_row(result)) {
                    AddListTill1(row[0], row[1], row[2], row[3], row[4]);
                    //printf("\t%s\t%s\t%s\t%s\t%s\n", row[0], row[1], row[2], row[3], row[4]);
                }
                ScanList1();
                FreeList1();

                printf("\t══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n");
                printf("\t请输入你要归还的书号，如果暂时不想还就输入 0 ：");
                scanf("%s", checkBorrowed_ret);

                //判断是不是非法输入，非法输入或者0直接返回
                while (1) {
                    if (strlen(checkBorrowed_ret) == 6) {
                        mysql_free_result(result);
                        mysql_close(&mysql);
                        return 1;
                    } else if (strcmp(checkBorrowed_ret, "0") == 0) {
                        mysql_free_result(result);
                        mysql_close(&mysql);
                        return 0;
                    } else {
                        printf("\t请输入你要操作的书号，如果暂时不想还就输入 0 ：");
                        scanf("%s", checkBorrowed_ret);
                        continue;
                    }
                }
            }
        }
    }
}

int returnBooks(char book_id[MAXLENGTH], char reader_id[MAXLENGTH]) {
    //printf("%s %s", book_id, reader_id);
    //getchar();
    //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("【保存书籍信息的数据库连不上了\n请按任意键结束");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        if (mysql_query(&mysql, "select * ,IF(return_plan < NOW(),1,0) from borrowed;")) {
            printf("\n\t【保存借阅信息的表格连不上了】\n请按任意键结束");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            //不存在借阅记录
            if (rowcount == 0) {
                printf("\n\t【看来你也没借书啊】\n按任意键回到上一级菜单！");
                //Sleep(1000);
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            }

            //不存在这本书的有效借阅记录，如果存在记录一下是否超期
            int penalty = 0;
            int flag = 1;

            while (row = mysql_fetch_row(result)) {
                //书号和用户名都对上了
                if (strcmp(row[0], book_id) == 0 && strcmp(row[1], reader_id) == 0) {
                    flag = 0;
                    //记录无效时相当于没记录
                    if (atoi(row[4]) == 0) {
                        flag = 1;
                        continue;
                    }
                        //否则先看看是不是超期了，超期了一会儿改表，同时有效记录变为无效
                    else {
                        penalty = atoi(row[5]);

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update borrowed set valid=0 where book_id = ");
                        strcat(query, "'");
                        strcat(query, book_id);
                        strcat(query, "' and reader_id ='");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        mysql_query(&mysql, query);
                    }
                }
            }

            if (flag) {
                printf("\n\t【看来你也没借这本书啊】\n按任意键回到上一级菜单！");
                //Sleep(1000);
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            }

            //读者表更改三个部分
            memset(query, 0, sizeof(char) * MAXLENGTH);
            strcat(query, "update reader set reader_have_borrowed=reader_have_borrowed-1 where reader_id = ");
            strcat(query, "'");
            strcat(query, reader_id);
            strcat(query, "';");
            mysql_query(&mysql, query);
            //printf("%s\n", query);
            //getchar();

            memset(query, 0, sizeof(char) * MAXLENGTH);
            strcat(query, "update reader set reader_left_borrowTimes=reader_left_borrowTimes+1 where reader_id = ");
            strcat(query, "'");
            strcat(query, reader_id);
            strcat(query, "';");
            mysql_query(&mysql, query);
            //printf("%s\n", query);
            //getchar();

            if (penalty) {
                memset(query, 0, sizeof(char) * MAXLENGTH);
                strcat(query, "update reader set reader_penalty=1 where reader_id = ");
                strcat(query, "'");
                strcat(query, reader_id);
                strcat(query, "';");
                mysql_query(&mysql, query);
                //printf("%s\n", query);
                //getchar();
            }

            //图书表借阅状态变为0
            memset(query, 0, sizeof(char) * MAXLENGTH);
            strcat(query, "update book set borrowed=0 where book_id = ");
            strcat(query, "'");
            strcat(query, book_id);
            strcat(query, "';");
            mysql_query(&mysql, query);
            //printf("%s\n", query);
            //getchar();

            //最后已还表添加内容
            memset(query, 0, sizeof(char) * MAXLENGTH);
            strcat(query, "INSERT INTO returned(book_id,reader_id,borrow_times,return_plan,return_real)\
				(SELECT book_id, reader_id, borrow_times, return_plan, NOW() FROM borrowed\
					WHERE book_id = '");
            strcat(query, book_id);
            strcat(query, "'AND reader_id = '");
            strcat(query, reader_id);
            strcat(query, "'); ");

            if (mysql_query(&mysql, query) != 0) //将dest插入到数据库中(db_books)
            {
                system("clear");
                fprintf(stderr, "【保存借阅信息的数据库连不上了】\n%s\n请按任意键结束",
                        mysql_error(&mysql));
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            } else {
                printf("\n\n\t\t【还书成功！！!】\n\t\t");
                getchar();
                mysql_free_result(result);    //释放结果集
                mysql_query(&mysql, "commit;");
                mysql_close(&mysql);          //释放连接
                return 0;
            }
        }
    }
}

//-------------------链表2
struct Node2 {
    //数据域
    char row0[MAXLENGTH];
    char row1[MAXLENGTH];
    char row2[MAXLENGTH];
    char row3[MAXLENGTH];
    char row4[MAXLENGTH];
    char row5[MAXLENGTH];
    char row6[MAXLENGTH];
    char row7[MAXLENGTH];
    char row8[MAXLENGTH];
    char row9[MAXLENGTH];
    char row10[MAXLENGTH];

    struct Node2 *next;    //指针域(指向节点的指针）
};
struct Node2 *head2 = NULL;
struct Node2 *end2 = NULL;

void
AddListTill2(char *row0, char *row1, char *row2, char *row3, char *row4, char *row5, char *row6, char *row7, char *row8,
             char *row9, char *row10) {
    //创建一个节点
    struct Node2 *temp = (struct Node2 *) malloc(sizeof(struct Node2));        //此处注意强制类型转换

    //节点数据进行赋值
    strcpy(temp->row0, row0);
    strcpy(temp->row1, row1);
    strcpy(temp->row2, row2);
    strcpy(temp->row3, row3);
    strcpy(temp->row4, row4);
    strcpy(temp->row5, row5);
    strcpy(temp->row6, row6);
    strcpy(temp->row7, row7);
    strcpy(temp->row8, row8);
    strcpy(temp->row9, row9);
    strcpy(temp->row10, row10);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head2) {

        head2 = temp;
        //	end=temp;
    } else {
        end2->next = temp;
        //	end=temp;			//尾结点应该始终指向最后一个
    }
    end2 = temp;            //尾结点应该始终指向最后一个
}

void ScanList2() {
    struct Node2 *temp = head2;        //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\n", temp->row0, temp->row1, temp->row2, temp->row3, temp->row4,
               temp->row5, temp->row6);
        printf("\n\n\t已借图书：%s本\t剩余借阅次数：%s次\t剩余续借次数：%s次\t是否罚金未清：%s\n", temp->row7, temp->row8, temp->row9, temp->row10);
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
    }
}

void FreeList2() {
    //一个一个NULL
    struct Node2 *temp = head2;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node2 *pt = temp;
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                    //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head2 = NULL;
    end2 = NULL;
}

//修改个人信息---查询个人信息+修改
int checkInfo() {
    //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("【保存信息的数据库连不上了】\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        memset(query, 0, sizeof(char) * MAXLENGTH);
        strcpy(query, "SELECT reader_id,reader_password,reader_name,IF(reader_sex=1,'男','女'),\
			(CASE reader_grade\
				WHEN 1 THEN '大一'\
				WHEN 2 THEN '大二'\
				WHEN 3 THEN '大三'\
				WHEN 4 THEN '大四'\
				WHEN 5 THEN '研究生'\
				WHEN 6 THEN '博士生'\
				WHEN 7 THEN '教师'\
				END), reader_telephone,\
				(CASE reader_department\
					WHEN 1 THEN '计算机学院'\
					WHEN 2 THEN '理学院'\
					WHEN 3 THEN '文法学院'\
					WHEN 4 THEN '生命科学学院'\
					END), `reader_have_borrowed`,`reader_left_borrowTimes`,\
			`reader_left_renewTimes`,IF(`reader_penalty`=1, '罚金未清', '无罚金')\
			FROM reader; ");


        if (mysql_query(&mysql, query)) {
            printf("\n\t【保存借阅人信息的表格连不上了】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\t════════════════════════════\n");
            printf("\t          个人信息一览          \n");
            printf("\t════════════════════════════\n");

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            printf("\t════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n");
            printf("\t1-用户名\t2-密码\t\t3-姓名\t\t4-性别\t\t5-身份\t\t6-电话号\t\t7-所属学院\n");
            while (row = mysql_fetch_row(result)) {
                if (strcmp(row[0], login_id) == 0) {
                    AddListTill2(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7], row[8], row[9],
                                 row[10]);
                    //printf("\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\n", row[0], row[1], row[2], row[3], row[4], row[5], row[6]);
                    //printf("\n\n\t已借图书：%s本\t剩余借阅次数：%s次\t剩余续借次数：%s次\t是否罚金未清：%s\n", row[7], row[8], row[9], row[10]);
                    ScanList2();
                    break;
                }
            }
            FreeList2();

            printf("\n\t══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n");
            printf("\t请输入你要修改的个人信息编号（2-7），【用户名不支持修改】，如果暂时不想修改就输入 0 吧：");

            char order[50] = {0};
            scanf("%s", order);

            //判断是不是非法输入，非法输入或者0直接返回
            if (strcmp(order, "2") == 0 || strcmp(order, "3") == 0 || strcmp(order, "4") == 0 ||
                strcmp(order, "5") == 0 || strcmp(order, "6") == 0 || strcmp(order, "7") == 0) {
                updateNum = atoi(order);
                //getchar();

                mysql_free_result(result);
                mysql_close(&mysql);
                return 0;
            } else {
                //getchar();
                mysql_free_result(result);
                mysql_close(&mysql);
                return 1;
            }
        }
    }
}

int updateInfo(int updateNum, char reader_id[MAXLENGTH]) {
    //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("【保存书籍信息的数据库连不上了】\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from reader")) {
            printf("\n\t【保存借阅人信息的表格连不上了】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            //switch case中不能定义数组。。。
            char infoLint[100] = "请输入学院【计算机学院 1，理学院 2，文法学院 3，生命科学学院 4】";
            while (1) {
                //1-用户名  2-密码  3-姓名  4-性别  5-身份  6-电话号  7-所属学院
                printf("\n\t================================\n");
                switch (updateNum) {
                    case 2:
                        //接着是密码，长度太长更换
                        while (1) {
                            printf("\n\t请输入密码【15位以内】: ");
                            scanf("%s", login_password);
                            if (strlen(login_password) > 15) {
                                printf("\t密码过长！");
                                continue;
                            } else {
                                break;
                            }
                        }

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_password = '");
                        strcat(query, login_password);
                        strcat(query, "'");
                        strcat(query, " where reader_id = '");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        //printf("%s\n", query);
                        if (mysql_query(&mysql, query)) {
                            printf("\n\t【保存借阅人信息的表格修改不了了】\n请按任意键结束这次不愉快的体验吧，对不起了");
                            getchar();
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);
                            return 1;
                        }
                        //getchar();
                        break;
                    case 3:
                        //接着是名字，长度太长更换
                        while (1) {
                            printf("\n\t请输入名字【15位以内】: ");
                            scanf("%s", login_name);
                            if (strlen(login_name) > 15) {
                                printf("\t名字过长！");
                                continue;
                            } else break;
                        }

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_name = '");
                        strcat(query, login_name);
                        strcat(query, "'");
                        strcat(query, " where reader_id = '");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        //printf("%s\n", query);
                        if (mysql_query(&mysql, query)) {
                            printf("\n\t【保存借阅人信息的表格修改不了了】\n请按任意键结束这次不愉快的体验吧，对不起了");
                            getchar();
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);
                            return 1;
                        }
                        //getchar();
                        break;
                    case 4:
                        //性别只能是1和2
                        while (1) {
                            printf("\n\t请输入性别【男1 女2】: ");
                            scanf("%s", login_gender);
                            if (strcmp(login_gender, "1") != 0 && strcmp(login_gender, "2") != 0) {
                                printf("\t输入有误!");
                                continue;
                            } else break;
                        }

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_sex = '");
                        strcat(query, login_gender);
                        strcat(query, "'");
                        strcat(query, " where reader_id = '");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        //printf("%s\n", query);
                        if (mysql_query(&mysql, query)) {
                            printf("\n\t【保存借阅人信息的表格修改不了了】\n请按任意键结束这次不愉快的体验吧，对不起了");
                            getchar();
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);
                            return 1;
                        }
                        //getchar();
                        break;
                    case 5:
                        //身份只能是1-7
                        while (1) {
                            printf("\n\t请输入身份【大一 1，大二 2，大三 3，大四 4，研究生 5，博士生 6，教师 7】: ");
                            scanf("%s", login_grade);
                            if (strcmp(login_grade, "1") != 0 && strcmp(login_grade, "2") != 0
                                && strcmp(login_grade, "3") != 0 && strcmp(login_grade, "4") != 0
                                && strcmp(login_grade, "5") != 0 && strcmp(login_grade, "6") != 0
                                && strcmp(login_grade, "7") != 0) {
                                printf("\t输入错误!");
                                continue;
                            } else break;
                        }

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_grade = '");
                        strcat(query, login_grade);
                        strcat(query, "'");
                        strcat(query, " where reader_id = '");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        //printf("%s\n", query);
                        if (mysql_query(&mysql, query)) {
                            printf("\n\t【保存借阅人信息的表格修改不了了】\n请按任意键结束这次不愉快的体验吧，对不起了");
                            getchar();
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);
                            return 1;
                        }
                        //getchar();
                        break;
                    case 6:
                        //手机号 也不能超过15位
                        while (1) {
                            printf("\n\t请输入手机号: ");
                            scanf("%s", login_phone);
                            if (strlen(login_phone) > 15) {
                                printf("\t输入有误！");
                                continue;
                            } else break;
                        }

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_phone = '");
                        strcat(query, login_phone);
                        strcat(query, "'");
                        strcat(query, " where reader_id = '");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        //printf("%s\n", query);
                        if (mysql_query(&mysql, query)) {
                            printf("\n\t【保存借阅人信息的表格修改不了了】\n请按任意键结束这次不愉快的体验吧，对不起了");
                            getchar();
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);
                            return 1;
                        }
                        //getchar();
                        break;
                    case 7:
                        //学院只能是1-4
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

                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_department = '");
                        strcat(query, login_department);
                        strcat(query, "'");
                        strcat(query, " where reader_id = '");
                        strcat(query, reader_id);
                        strcat(query, "';");
                        //printf("%s\n", query);
                        if (mysql_query(&mysql, query)) {
                            printf("\n\t【保存借阅人信息的表格修改不了了】\n请按任意键结束这次不愉快的体验吧，对不起了");
                            getchar();
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);
                            return 1;
                        }
                        //getchar();
                        break;
                }

                mysql_query(&mysql, "commit;");
                printf("\n\t【修改成功！】\n");
                printf("\n\t你还想改别的吗？输入0退出修改，输入其他内容继续：");

                char order[50] = {0};
                scanf("%s", order);

                //判断是不是非法输入，非法输入或者0直接返回
                if (strcmp(order, "0") != 0) {
                    system("clear");
                    if (result != NULL)mysql_free_result(result);
                    mysql_close(&mysql);
                    return 2;
                } else {
                    if (result != NULL)mysql_free_result(result);
                    //mysql_query(&mysql, "commit;");
                    mysql_close(&mysql);
                    return 0;
                }
            }
        }
    }
}

//申请续借
int checkRenew() {
    //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("【保存书籍信息的数据库连不上了】\n请按任意键结束...");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        if (mysql_query(&mysql, "select * from borrowed")) {
            printf("\n\t【保存借阅人信息的表格连不上了】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\t════════════════════════════\n");
            printf("\t          查询借阅情况          \n");
            printf("\t════════════════════════════\n");

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            if (rowcount == 0) {
                printf("\n\t【看来你还没借书啊，先去借一本书再来吧】\n\t回到上一级菜单借本书！\n\n\t");
                //Sleep(1000);
                getchar();
                mysql_free_result(result);
                mysql_close(&mysql);
                return 1;
            } else {
                mysql_free_result(result);
                mysql_query(&mysql, "SELECT b.book_id,book_name,borrow_times,return_plan,IF(return_plan<NOW(),'超期有罚金','未超期') penalty\
					FROM borrowed bd\
					INNER JOIN book b\
					ON bd.`book_id` = b.`book_id`\
					where valid=1;");

                result = mysql_store_result(&mysql);//获得结果集
                rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

                printf("\t════════════════════════════════════════════════════════════════════════════════════\n");
                printf("\t书号\t书名\t\t\t借书时间\t\t应还时间\t\t罚金\n");
                while (row = mysql_fetch_row(result)) {
                    printf("\t%s\t%-20s\t%-15s\t%-15s\t%s\n", row[0], row[1], row[2], row[3], row[4]);
                }

                printf("\n\n\t════════════════════════════════════════════════════════════════════════════════════\n");
                printf("\t请输入你要续借的书号，如果暂时不想续借就输入 0 吧：");
                scanf("%s", checkBorrowed_ret);

                //判断是不是非法输入，非法输入或者0直接返回
                if (strlen(checkBorrowed_ret) == 6) {
                    mysql_free_result(result);
                    mysql_close(&mysql);
                    return 0;
                } else {
                    mysql_free_result(result);
                    mysql_close(&mysql);
                    return 1;
                }
            }
        }
    }
}

int renew(char book_id[MAXLENGTH], char reader_id[MAXLENGTH]) {
    //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("【保存书籍信息的数据库连不上了】\n请按任意键结束...");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * ,IF(return_plan < NOW(), 1, 0) from borrowed;")) {
            printf("\n\t【保存借阅信息的表格连不上了】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\t════════════════════════════\n");
            printf("\t          续借图书          \n");
            printf("\t════════════════════════════\n");

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            //不存在借阅记录
            if (rowcount == 0) {
                printf("\n\t【看来你也没借书啊】\n\t按任意键回到上一级菜单！\n\t");
                //Sleep(1000);
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            }

            //不存在这本书的有效借阅记录，如果存在记录一下是否超期
            int penalty = 0;
            int flag = 1;

            while (row = mysql_fetch_row(result)) {
                //书号和用户名都对上了
                if (strcmp(row[0], book_id) == 0 && strcmp(row[1], reader_id) == 0) {
                    flag = 0;
                    //记录无效时相当于没记录
                    if (atoi(row[4]) == 0) {
                        flag = 1;
                        continue;
                    }
                        //否则先看看是不是超期了，超期了一会儿改表
                    else {
                        penalty = atoi(row[5]);
                    }
                }
            }

            if (flag) {
                printf("\n\t【看来你也没借这本书啊】\n\t按任意键回到上一级菜单！\n\t");
                //Sleep(1000);
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            }

            printf("\n\t【续借图书可以延长应还日期 30 天，但若已经超期，罚金仍然生效，罚金存在时不可再借】\n\t");

            //读者表罚金一栏
            if (penalty) {
                memset(query, 0, sizeof(char) * MAXLENGTH);
                strcat(query, "update reader set penalty=1 where reader_id = ");
                strcat(query, "'");
                strcat(query, reader_id);
                strcat(query, "';");
                mysql_query(&mysql, query);
                printf("%s\n", query);
                getchar();
            }


            //最后借阅表应还时间更改
            //拼接语句
            memset(query, 0, sizeof(char) * MAXLENGTH);
            strcat(query, "update borrowed set return_plan=DATE_SUB(return_plan, INTERVAL -15 DAY) where book_id = ");
            strcat(query, "'");
            strcat(query, book_id);
            strcat(query, "' and reader_id ='");
            strcat(query, reader_id);
            strcat(query, "';");
            mysql_query(&mysql, query);

            if (mysql_query(&mysql, query) != 0) //修改看是否成功
            {
                system("clear");
                fprintf(stderr, "【保存借阅信息的数据库连不上了】\n%s\n请按任意键结束这次不愉快的体验吧，对不起了",
                        mysql_error(&mysql));
                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            } else {
                printf("\n\n\t【续借成功！！!】\n\t");
                sleep(800);
                getchar();
                mysql_free_result(result);    //释放结果集
                mysql_query(&mysql, "commit;");
                mysql_close(&mysql);          //释放连接
                return 0;
            }
        }
    }
}

//交延期罚金
int payDelayPenalty(char reader_id[MAXLENGTH]) {
    //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");

        memset(query, 0, sizeof(char) * MAXLENGTH);
        strcat(query, "select * from reader where reader_id = '");
        strcat(query, reader_id);
        strcat(query, "';");
        //printf("%s\n", query);

        if (mysql_query(&mysql, query)) {
            printf("\n\t保存借阅人信息的表格连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\t════════════════════════════\n");
            printf("\t          延期罚金结清          \n");
            printf("\t════════════════════════════\n");

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            row = mysql_fetch_row(result);

            //没有罚金时
            if (strcmp(row[10], "0") == 0) {
                printf("\n\n\t你没有罚金未结清啊，尽情享受阅读吧！\n\t");

                getchar();
                mysql_free_result(result);
                mysql_query(&mysql, "commit;");
                mysql_close(&mysql);
                return 0;
            }

            printf("\n\n\t【这里只能结清你的延期罚金【5元】...】\n\n");
            printf("\t【为了您的账号安全，您只有3次输入账号和密码的机会，三次全部错误，则锁定账号...】\n");
            mysql_free_result(result);    //释放结果集

            int flag = 1;
            for (int i = 0; i < 3; ++i) {
                printf("\n\t请输入您的校园卡账号：");
                scanf("%s", school_id);
                printf("\t请输入您的校园卡密码：");
                //星号密码显示部分
                char c = '\0';
                int i = 0;
                initscr(); // getch()使用前的初始化
                while ((c = getch()) != '\r') {
                    if (i < MAXLENGTH && isprint(c)) {
                        school_password[i++] = c;
                        putchar('*');
                    } else if (i > 0 && c == '\b') {
                        --i;
                        putchar('\b');
                        putchar(' ');
                        putchar('\b');
                    }
                }
                endwin(); // getch()使用后的注销
                putchar('\n');
                school_password[i] = '\0';

                memset(query, 0, sizeof(char) * MAXLENGTH);
                strcat(query, "select * from schoolcard where school_id = '");
                strcat(query, school_id);
                strcat(query, "';");

                if (mysql_query(&mysql, query)) {
                    printf("\n\t【保存校园卡信息的表格连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
                    getchar();
                    mysql_query(&mysql, "rollback;");
                    mysql_close(&mysql);
                    return 1;
                } else {
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) {
                        printf("\n\t【校园卡号不存在】\n\t");
                        //sleep(1000);
                        getchar();
                        mysql_free_result(result);
                        mysql_query(&mysql, "rollback;");
                        continue;
                    } else {
                        row = mysql_fetch_row(result);
                        //没有这个id
                        //printf("%s %s %s %s\n", row[0], row[1], row[2], row[3]);
                        if (strcmp(row[1], school_password) != 0) {
                            //printf("%s %s\n", row[1], school_password);
                            printf("\n\t【校园卡密码错误】\n\t");
                            //Sleep(1000);
                            getchar();
                            mysql_free_result(result);
                            mysql_query(&mysql, "rollback;");
                            continue;
                        } else if (strcmp(row[3], "0") != 0) {
                            printf("\n\t【校园卡已被锁定，不能访问】\n\t");
                            //Sleep(1000);
                            getchar();
                            mysql_free_result(result);
                            mysql_query(&mysql, "rollback;");
                            mysql_close(&mysql);          //释放连接
                            return 1;
                        } else if (atof(row[2]) < 5) {
                            printf("\n\t【校园卡余额不足，最起码要5块钱才可以！】\n\t");
                            //Sleep(1000);
                            getchar();
                            mysql_free_result(result);
                            mysql_query(&mysql, "rollback;");
                            mysql_query(&mysql, "rollback;");
                            return 1;
                        } else { // 成功
                            flag = 0;

                            //修改个人罚金为0
                            memset(query, 0, sizeof(char) * MAXLENGTH);
                            strcat(query, "update reader set reader_penalty=0 where reader_id = ");
                            strcat(query, "'");
                            strcat(query, reader_id);
                            strcat(query, "';");
                            mysql_query(&mysql, query);
                            //printf("%s\n", query);
                            //getchar();

                            //校园卡余额-5
                            memset(query, 0, sizeof(char) * MAXLENGTH);
                            strcat(query, "update schoolcard set left_money=left_money-5 where school_id = ");
                            strcat(query, "'");
                            strcat(query, school_id);
                            strcat(query, "';");
                            mysql_query(&mysql, query);
                            //printf("%s\n", query);
                            //getchar();

                            break;
                        }
                    }

                }
            }

            if (flag == 0) {
                printf("\n\n\t【罚金已经结清！！!】\n");
                sleep(800);
                mysql_free_result(result);    //释放结果集
                mysql_query(&mysql, "commit;");
                mysql_close(&mysql);          //释放连接
                return 0;
            } else {
                printf("\n\n\t【错误尝试过多！！!】\n");
                sleep(800);
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);          //释放连接
                return 1;
            }
        }
    }
}

//-------------------链表7
struct Node7 {
    //数据域
    char row0[MAXLENGTH];
    char row1[MAXLENGTH];
    char row2[MAXLENGTH];
    char row3[MAXLENGTH];
    char row4[MAXLENGTH];

    struct Node7 *next;    //指针域(指向节点的指针）
};
struct Node7 *head7 = NULL;
struct Node7 *end7 = NULL;

void AddListTill7(char *row0, char *row1, char *row2, char *row3, char *row4) {
    //创建一个节点
    struct Node7 *temp = (struct Node7 *) malloc(sizeof(struct Node7));        //此处注意强制类型转换

    //节点数据进行赋值
    strcpy(temp->row0, row0);
    strcpy(temp->row1, row1);
    strcpy(temp->row2, row2);
    strcpy(temp->row3, row3);
    strcpy(temp->row4, row4);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head7) {
        head7 = temp;
        //	end=temp;
    } else {
        end7->next = temp;
        //	end=temp;			//尾结点应该始终指向最后一个
    }
    end7 = temp;            //尾结点应该始终指向最后一个
}

void ScanList7() {
    struct Node7 *temp = head7;        //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("\t%s\t\t%-30s\t%-30s\t%-30s\t%s\n", temp->row0, temp->row1, temp->row2, temp->row3, temp->row4);
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
    }
}

void FreeList7() {
    //一个一个NULL
    struct Node7 *temp = head7;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node7 *pt = temp;
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                    //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head7 = NULL;
    end7 = NULL;
}

//查看已还信息
int borrowinfoSelf() {
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        if (mysql_query(&mysql, "select * from borrowed")) {
            printf("\n\t保存借阅人信息的表格连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            system("clear");
            memset(query, 0, sizeof(char) * MAXLENGTH);
            strcat(query, "SELECT bo.book_id,book_name,borrow_times,return_real,IF(return_plan>return_real,'未超期','超期') FROM returned bo INNER JOIN book b\
				ON bo.`book_id` = b.`book_id` WHERE bo.reader_id = '");
            strcat(query, login_id);
            strcat(query, "';");
            //printf("%s", query);
            if (mysql_query(&mysql, query)) {
                printf("\n\t【错误：%s】\n请按任意键结束这次不愉快的体验吧，对不起了", mysql_error(&mysql));
                getchar();
                mysql_close(&mysql);
                return 1;
            }
            //eymysql_free_result(result);
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            if (rowcount == 0) {
                printf("\n\t您没有还书记录！");
            } else {
                printf("\n\n\n");
                printf("\t%s\t%-30s\t%-30s\t%-30s\t%s\n", "书号", "书名", "借书时间", "应还时间", "是否超期");
                printf("\t═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n");
                while (row = mysql_fetch_row(result)) {
                    AddListTill1(row[0], row[1], row[2], row[3], row[4]);
                    //printf("\t%s\t%s\t%s\t%s\t%s\n", row[0], row[1], row[2], row[3], row[4]);
                }
                ScanList1();
                FreeList1();
            }
            printf("\n\n\t");
            getchar();
        }
        mysql_query(&mysql, "commit;");
        mysql_close(&mysql); //释放连接
    }
}


//////////////管理员的函数功能部分///////////////////////
//-------------------链表3
struct Node3 {
    //数据域
    char row0[MAXLENGTH];
    char row1[MAXLENGTH];

    struct Node3 *next;    //指针域(指向节点的指针）
};

struct Node3 *head3 = NULL;
struct Node3 *end3 = NULL;

void AddListTill3(char *row0, char *row1) {
    //创建一个节点
    struct Node3 *temp = (struct Node3 *) malloc(sizeof(struct Node3));        //此处注意强制类型转换

    //节点数据进行赋值
    strcpy(temp->row0, row0);

    strcpy(temp->row1, row1);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head3) {
        head3 = temp;
        //	end=temp;
    } else {
        end3->next = temp;
        //	end=temp;			//尾结点应该始终指向最后一个
    }
    end3 = temp;            //尾结点应该始终指向最后一个
}

void ScanList3() {
    struct Node3 *temp = head3;        //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("%-10s\t\t\t%-10s\t\t║\n", temp->row0, temp->row1);
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
    }

}

void FreeList3() {
    //一个一个NULL
    struct Node3 *temp = head3;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node3 *pt = temp;
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                    //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head3 = NULL;
    end3 = NULL;
}

//查询个人信息
int SecPi() {
    //首先连接数据库
    mysql_init(&mysql);
    if (!(mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0))) {
        printf("保存管理员信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到管理员的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        memset(query, 0, sizeof(char) * MAXLENGTH);
        strcat(query, "select * from admin where admin_id = '");
        strcat(query, login_id);
        strcat(query, "'");
        //printf("%s", query);
        if (mysql_query(&mysql, query)) {
            printf("\n\t保存管理员信息的表格连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            if (mysql_num_rows(result) != 0) {  //获得结果集的行数，若行数不为0，则进入if
                //有记录数据时，才显示记录数据
                printf("\n\n");
                printf("\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
                printf("\t║\t\t\t\t 显 示 个 人 信 息\t\t\t\t║\n");
                printf("\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
                printf("\t║\t\t\t☆ 管理员 ID\t\t☆ 管理员密码\t\t\t║\n");
                printf("\t║\t\t\t     ");
                while (row = mysql_fetch_row(result)) //取出结果集中的下一行数据
                {
                    AddListTill3(row[0], row[1]);
                    ScanList3();
                    FreeList3();
                    //取出结果集中记录
                    //fprintf(stdout, "%s\t\t\t\t%s", row[0], row[1]);  //输出这行记录 [相当于从文件中取出数据，所以使用了fprintf]
                }
                printf("\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
                printf("\n\n\t");

            } else {
                printf("\n\t没有记录数据! \n");
            }
            mysql_free_result(result); //释放结果集
        }

        mysql_close(&mysql); //释放连接
    }
    getchar();
    return 1;
}

//修改个人信息
int ModifyPi() {
    char *sql;
    char dest[500] = {"  "};
    char *admin_id;
    char *admin_password;

    //首先连接数据库

    mysql_init(&mysql);
    if (!(mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0))) {
        printf("保存管理员信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到管理员的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        memset(query, 0, sizeof(char) * MAXLENGTH);
        strcat(query, "select * from admin where admin_id = '");
        strcat(query, login_id);
        strcat(query, "'");
        //printf("%s", query);
        if (mysql_query(&mysql, query)) {
            printf("\n\t保存管理员信息的表格连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            /*数据库连接成功*/
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            if (mysql_num_rows(result) != 0) //获得结果集的行数，若行数不为0，则进入if
            {
                //有记录数据时，才显示记录数据
                printf("\n\n\n\n\n\n");
                printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
                printf("\t\t\t║\t\t\t\t 显 示 个 人 信 息\t\t\t\t║\n");
                printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
                printf("\t\t\t║\t\t\t☆ 管理员 ID\t\t☆ 管理员密码\t\t\t║\n");
                printf("\t\t\t║\t\t\t     ");
                while (row = mysql_fetch_row(result)) //取出结果集中的下一行数据
                {
                    //取出结果集中记录
                    fprintf(stdout, "%-20s%-20s", row[0],
                            row[1]);  //输出这行记录 [相当于从文件中取出数据，所以使用了fprintf]
                }
                printf("\t\t║\n");
                printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
                printf("\n");
                printf("\t     ☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆是否修改信息★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
                printf("\t\t\t                       ╔══════════════╦══════════════╗                       \n");
                printf("\t\t\t                       ║  ☆ Y. 是    ║  ☆ N.否     ║\n");
                printf("\t\t\t                       ╚══════════════╩══════════════╝\n");
                while (1) {
                    printf("\n\t\t\t\t\t\t\t\t");
                    scanf("%s", ch);

                    if (strcmp(ch, "Y") == 0 || strcmp(ch, "y") == 0 || strcmp(ch, "N") == 0 ||
                        strcmp(ch, "n") == 0)
                        break;
                    else {
                        printf("\n\t\t\t\t\t\t\t\t\t修改请输入【Y】/不修改请输入【N】\n");
                    }
                }
                if (strcmp(ch, "Y") == 0 || strcmp(ch, "y") == 0)    /*判断是否需要录入*/
                {

                    sql = "update admin set admin_id= '";
                    strcat(dest, sql);

                    admin_id = (char *) malloc(20);
                    admin_password = (char *) malloc(20);

                    printf("\t\t\t\t 新ID:");
                    scanf("%s", admin_id);   /*输入新ID*/
                    printf("\n");
                    strcat(dest, admin_id);
                    printf("\t\t\t\t 新密码:");
                    scanf("%s", admin_password);    /*输入新密码*/
                    strcat(dest, "', admin_password = '");
                    strcat(dest, admin_password);
                    strcat(dest, "';");


                    if (mysql_query(&mysql, dest) != 0) {

                        printf("\n\n\t\t\t\t\t\t☆★☆★☆★【修改失败】★☆★☆★☆");
                        printf("\n\n\t\t\t\t");
                        mysql_query(&mysql, "rollback;");
                    } else {
                        printf("\n\n\t\t\t\t\t\t☆★☆★☆★【修改成功】★☆★☆★☆");
                        printf("\n\n\t\t\t\t\t\t\t");
                        mysql_query(&mysql, "commit;");
                    }

                }
            }
            mysql_free_result(result);          //释放结果集
        }
        mysql_close(&mysql);     //释放连接
    }
    printf("\n\n\t\t\t\t\t\t\t");
    getchar();
    return 1;
}

//-------------------链表4
void
AddListTill4(char *row0, char *row1, char *row2, char *row3, char *row4, char *row5, char *row6, char *row7, char *row8,
             char *row9, char *row10) {
    //创建一个节点
    struct Node4 *temp = (struct Node4 *) malloc(sizeof(struct Node4));        //此处注意强制类型转换

    //节点数据进行赋值
    strcpy(temp->row0, row0);
    strcpy(temp->row1, row1);
    strcpy(temp->row2, row2);
    strcpy(temp->row3, row3);
    strcpy(temp->row4, row4);
    strcpy(temp->row5, row5);
    strcpy(temp->row6, row6);
    strcpy(temp->row7, row7);
    strcpy(temp->row8, row8);
    strcpy(temp->row9, row9);
    strcpy(temp->row10, row10);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head4) {

        head4 = temp;
        //	end=temp;
    } else {
        end4->next = temp;
        //	end=temp;			//尾结点应该始终指向最后一个
    }
    end4 = temp;            //尾结点应该始终指向最后一个
}

struct Node4 *head4 = NULL;
struct Node4 *end4 = NULL;

void ScanList4() {
    struct Node4 *temp = head4;        //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("\n\t\t\t║ ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s ║", temp->row0, temp->row1, temp->row2,
               temp->row3, temp->row4, temp->row5, temp->row6);
        printf("\n\t\t\t║\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t ║");
        printf("\n\t\t\t║图书简介:\t%-100s\t ║", temp->row7);
        printf("\n\t\t\t║\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t ║");
        printf("\n\t\t\t║图书类别: %-20s\t\t馆藏总数: %-10s\t\t馆藏现存: %-10s\t\t\t\t ║", temp->row8, temp->row9, temp->row10);
        printf("\n\t\t\t╠════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣");
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
    }

}

void FreeList4() {
    //一个一个NULL
    struct Node4 *temp = head4;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node4 *pt = temp;
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                    //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head4 = NULL;
    end4 = NULL;
}

//查询图书信息
int Checkbook(int flag) {
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到图书的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from book")) {
            printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            while (1) {
                system("cls");
                bookinforMenu2();

                scanf("%s", type1);
                if (strcmp(type1, "1") == 0) {
                    printf("\n\t\t\t 请输入作者名：");
                    scanf("%s", author);
                    printf("\n");
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
						FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
						GROUP BY isbn) finalBook WHERE writer LIKE '%");
                    strcat(query, author);
                    strcat(query, "%';");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //mysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    printf("\n\t\t\t一共找到%d本书\n\n", rowcount);
                    if (rowcount == 0) {
                        printf("输入的作者不存在哦；");
                        getchar();
                        continue;
                    } else {
                        printf("\t\t\t╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
                        //printf("\t\t\t║    书号 \t\t  书名  \t\t      图书号\t\t作者  \t  价钱  \t出版社  \t 书架号  ║");
                        printf("\t\t\t║ ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s║", "书号", "书名", "图书号", "作者",
                               "价格", "出版社", "书架号");
                        while (row = mysql_fetch_row(result)) {
                            AddListTill4(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[8], row[10],
                                         row[11], row[12]);
                            ScanList4();
                            FreeList4();
                        }

                    }

                    if (flag == 1) {
                        printf("\n\n\t\t\t════════════════════════════════════════════════════════════════════════════════════\n");
                        printf("\t\t\t请输入你要借阅的书号，如果暂时不想借就输入 0 吧：");
                        scanf("%s", book_id_next);

                        //判断是不是非法输入，非法输入或者0直接返回
                        while (1) {
                            if (strlen(book_id_next) == 6) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 1;
                            } else if (strcmp(book_id_next, "0") == 0) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 0;
                            } else {
                                printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                                scanf("%s", book_id_next);
                                continue;
                            }
                        }
                    }
                    if (flag == 2) {


                        printf("\n\t\t\t     ☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
                        printf("\t\t\t\t                       ╔══════════════╦══════════════╗                       \n");
                        printf("\t\t\t\t                       ║  ☆ 1. 删除  ║  ☆ 2.修改   ║\n");
                        printf("\t\t\t\t                       ╚═╦════════════╩════════════╦═╝\n");
                        printf("\t\t\t\t                         ║    ☆ 0. 不做任何处理   ║\n");
                        printf("\t\t\t\t                         ╚═════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                if (Deletebook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '2') {
                                if (Modifybook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    if (flag == 3) {


                        printf("\n\t\t\t╔══════════════════════════════════════════╦══════════════════════════════════════════╗                       \n");
                        printf("\t\t\t║  ☆ 1. 已经查找已经找到所要查看的图书    ║            ☆ 0.返回继续查找按           ║\n");
                        printf("\t\t\t╚══════════════════════════════════════════╩══════════════════════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                //mysql_close(&mysql);
                                return 0;
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }

                    printf("\n\t\t\t\t");
                    getchar();
                    continue;
                } else if (strcmp(type1, "2") == 0) {
                    printf("\n\t\t\t 请输入图书类别：");
                    scanf("%s", bookclass);
                    printf("\n");
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
						FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
						GROUP BY isbn) finalBook WHERE class LIKE '%");
                    strcat(query, bookclass);
                    strcat(query, "%';");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //mysql_free_result(result);

                    result = mysql_store_result(&mysql);//获
                    rowcount = (int) mysql_num_rows(result);//获得结果集得结果集有多少行
                    if (rowcount == 0) {
                        printf("输入的类别不存在哦，请重新输入；");
                        getchar();
                        continue;
                    } else {
                        printf("\t\t\t╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
                        printf("\t\t\t║ ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s║", "书号", "书名", "图书号", "作者",
                               "价格", "出版社", "书架号");
                        while (row = mysql_fetch_row(result)) {
                            AddListTill4(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[8], row[10],
                                         row[11], row[12]);
                            ScanList4();
                            FreeList4();
                        }
                    }

                    if (flag == 1) {
                        printf("\n\n\t\t\t════════════════════════════════════════════════════════════════════════════════════\n");
                        printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                        scanf("%s", book_id_next);

                        //判断是不是非法输入，非法输入或者0直接返回
                        while (1) {
                            if (strlen(book_id_next) == 6) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 1;
                            } else if (strcmp(book_id_next, "0") == 0) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 0;
                            } else {
                                printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                                scanf("%s", book_id_next);
                                continue;
                            }
                        }
                    }
                    if (flag == 2) {


                        printf("\n\t\t\t     ☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
                        printf("\t\t\t\t                       ╔══════════════╦══════════════╗                       \n");
                        printf("\t\t\t\t                       ║  ☆ 1. 删除  ║  ☆ 2.修改   ║\n");
                        printf("\t\t\t\t                       ╚═╦════════════╩════════════╦═╝\n");
                        printf("\t\t\t\t                         ║    ☆ 0. 不做任何处理   ║\n");
                        printf("\t\t\t\t                         ╚═════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                if (Deletebook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '2') {
                                if (Modifybook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    if (flag == 3) {


                        printf("\n\t\t\t\t\t\t╔══════════════════════════════════════════╦══════════════════════════════════════════╗                       \n");
                        printf("\t\t\t\t\t\t║  ☆ 1. 已经查找已经找到所要查看的图书    ║            ☆ 0.返回继续查找按           ║\n");
                        printf("\t\t\t\t\t\t╚══════════════════════════════════════════╩══════════════════════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                //mysql_close(&mysql);
                                return 0;
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }

                    printf("\n\t\t\t\t");
                    getchar();
                    continue;
                } else if (strcmp(type1, "3") == 0) {
                    printf("\n\t\t\t 请输入书名：");
                    scanf("%s", bookname);
                    printf("\n");
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
					FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
					GROUP BY isbn) finalBook WHERE book_name LIKE '%");
                    strcat(query, bookname);
                    strcat(query, "%';");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);

                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) {
                        printf("输入的图书名不存在哦；");
                        getchar();
                        continue;
                    } else {
                        printf("\t\t\t╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
                        printf("\t\t\t║ ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s║", "书号", "书名", "图书号", "作者",
                               "价格", "出版社", "书架号");
                        while (row = mysql_fetch_row(result)) {
                            AddListTill4(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[8], row[10],
                                         row[11], row[12]);
                            ScanList4();
                            FreeList4();
                        }
                    }

                    if (flag == 1) {
                        printf("\n\n\t\t\t════════════════════════════════════════════════════════════════════════════════════\n");
                        printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                        scanf("%s", book_id_next);

                        //判断是不是非法输入，非法输入或者0直接返回
                        while (1) {
                            if (strlen(book_id_next) == 6) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 1;
                            } else if (strcmp(book_id_next, "0") == 0) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 0;
                            } else {
                                printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                                scanf("%s", book_id_next);
                                continue;
                            }
                        }
                    }
                    if (flag == 2) {


                        printf("\n\t\t\t     ☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
                        printf("\t\t\t\t                       ╔══════════════╦══════════════╗                       \n");
                        printf("\t\t\t\t                       ║  ☆ 1. 删除  ║  ☆ 2.修改   ║\n");
                        printf("\t\t\t\t                       ╚═╦════════════╩════════════╦═╝\n");
                        printf("\t\t\t\t                         ║    ☆ 0. 不做任何处理   ║\n");
                        printf("\t\t\t\t                         ╚═════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                if (Deletebook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '2') {
                                if (Modifybook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    if (flag == 3) {


                        printf("\n\t\t\t\t\t\t╔══════════════════════════════════════════╦══════════════════════════════════════════╗                       \n");
                        printf("\t\t\t\t\t\t║  ☆ 1. 已经查找已经找到所要查看的图书    ║            ☆ 0.返回继续查找按           ║\n");
                        printf("\t\t\t\t\t\t╚══════════════════════════════════════════╩══════════════════════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                //mysql_close(&mysql);
                                return 0;
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    printf("\n\t\t\t\t");
                    getchar();
                    continue;
                } else if (strcmp(type1, "4") == 0) {
                    printf("\n\t\t\t 请输入图书号：");
                    scanf("%s", isbn);
                    printf("\n");
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
					FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
					GROUP BY isbn) finalBook WHERE isbn LIKE '%");
                    strcat(query, isbn);
                    strcat(query, "%';");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);

                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) {
                        printf("输入的图书号不存在哦；");
                        getchar();
                        continue;
                    } else {
                        printf("\t\t\t╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
                        printf("\t\t\t║ ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s║", "书号", "书名", "图书号", "作者",
                               "价格", "出版社", "书架号");
                        while (row = mysql_fetch_row(result)) {
                            AddListTill4(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[8], row[10],
                                         row[11], row[12]);
                            ScanList4();
                            FreeList4();
                        }
                    }

                    if (flag == 1) {
                        printf("\n\n\t\t\t════════════════════════════════════════════════════════════════════════════════════\n");
                        printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                        scanf("%s", book_id_next);

                        //判断是不是非法输入，非法输入或者0直接返回
                        while (1) {
                            if (strlen(book_id_next) == 6) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 1;
                            } else if (strcmp(book_id_next, "0") == 0) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 0;
                            } else {
                                printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                                scanf("%s", book_id_next);
                                continue;
                            }
                        }
                    }
                    if (flag == 2) {

                        printf("\n\t\t\t     ☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
                        printf("\t\t\t\t                       ╔══════════════╦══════════════╗                       \n");
                        printf("\t\t\t\t                       ║  ☆ 1. 删除  ║  ☆ 2.修改   ║\n");
                        printf("\t\t\t\t                       ╚═╦════════════╩════════════╦═╝\n");
                        printf("\t\t\t\t                         ║    ☆ 0. 不做任何处理   ║\n");
                        printf("\t\t\t\t                         ╚═════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                if (Deletebook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '2') {
                                if (Modifybook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    if (flag == 3) {


                        printf("\n\t\t\t\t\t\t╔══════════════════════════════════════════╦══════════════════════════════════════════╗                       \n");
                        printf("\t\t\t\t\t\t║  ☆ 1. 已经查找已经找到所要查看的图书    ║            ☆ 0.返回继续查找按           ║\n");
                        printf("\t\t\t\t\t\t╚══════════════════════════════════════════╩══════════════════════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                //mysql_close(&mysql);
                                return 0;
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    printf("\n\t\t\t\t");
                    getchar();
                    continue;
                } else if (strcmp(type1, "5") == 0) {
                    printf("\n\t\t\t 以下为本馆所有图书：\n");


                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
					FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
					GROUP BY isbn) finalBook； ");

                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);

                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) {
                        printf("这个破馆子没书了！");
                        getchar();
                        continue;
                    } else {
                        printf("\t\t\t╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
                        printf("\t\t\t║ ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s║", "书号", "书名", "图书号", "作者",
                               "价格", "出版社", "书架号");
                        while (row = mysql_fetch_row(result)) {
                            AddListTill4(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[8], row[10],
                                         row[11], row[12]);
                            ScanList4();
                            FreeList4();
                        }
                    }

                    if (flag == 1) {
                        printf("\n\n\t\t\t════════════════════════════════════════════════════════════════════════════════════\n");
                        printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                        scanf("%s", book_id_next);

                        //判断是不是非法输入，非法输入或者0直接返回
                        while (1) {
                            if (strlen(book_id_next) == 6) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 1;
                            } else if (strcmp(book_id_next, "0") == 0) {
                                mysql_free_result(result);
                                mysql_close(&mysql);
                                return 0;
                            } else {
                                printf("\t\t\t请输入你要操作的书号，如果暂时不想还就输入 0 吧：");
                                scanf("%s", book_id_next);
                                continue;
                            }
                        }
                    }
                    if (flag == 2) {


                        printf("\n\t\t\t     ☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
                        printf("\t\t\t\t                       ╔══════════════╦══════════════╗                       \n");
                        printf("\t\t\t\t                       ║  ☆ 1. 删除  ║  ☆ 2.修改   ║\n");
                        printf("\t\t\t\t                       ╚═╦════════════╩════════════╦═╝\n");
                        printf("\t\t\t\t                         ║    ☆ 0. 不做任何处理   ║\n");
                        printf("\t\t\t\t                         ╚═════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                if (Deletebook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '2') {
                                if (Modifybook() == 2) {
                                    mysql_close(&mysql);
                                    return 3;
                                } else {
                                    mysql_close(&mysql);
                                    return 0;
                                }
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    if (flag == 3) {


                        printf("\n\t\t\t\t\t\t╔══════════════════════════════════════════╦══════════════════════════════════════════╗                       \n");
                        printf("\t\t\t\t\t\t║  ☆ 1. 已经查找已经找到所要查看的图书    ║            ☆ 0.返回继续查找按           ║\n");
                        printf("\t\t\t\t\t\t╚══════════════════════════════════════════╩══════════════════════════════════════════╝\n");
                        char a;
                        while (1) {
                            a = getchar();
                            if (a == '1') {
                                //mysql_close(&mysql);
                                return 0;
                            } else if (a == '0') {
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                    printf("\n\t\t\t\t");
                    getchar();
                    continue;
                } else if (strcmp(type1, "0") == 0) {
                    //mysql_free_result(result);
                    mysql_query(&mysql, "commit;");
                    mysql_close(&mysql); //释放连接
                    break;
                } else {
                    continue;
                }
            }
            mysql_close(&mysql); //释放连接
            return 0;
        }
    }

}

//增添图书
int Addbook() {
    mysql_init(&mysql);
    if (!(mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0))) {
        printf("【震惊！保存图书的数据库连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到管理员的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from bookshelf;")) {
            printf("\n\t【震惊！保存书架信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            int i;
            int nBook = 1;
            char j, k[10];

            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            while (1) {
                //有记录数据时，才显示记录数据
                system("cls");
                printf("\t ════════════════════════════ \n");
                printf("\t       添 加 图 书 信 息                  \n");
                printf("\t ════════════════════════════ \n");


                memset(query, 0, sizeof(char) * MAXLENGTH);
                printf("\t书名:");
                scanf("%s", bookname);

                printf("\n\t图书isbn:");
                scanf("%s", isbn);

                printf("\n\t作者:");
                scanf("%s", writer);

                printf("\n\t价格:");
                while (1) {
                    if (scanf("%lf", &priceUser) == 1) {
                        //printf("\n%lf\n", priceUser);
                        break;
                    }
                    char c_tmp;
                    while ((c_tmp = getchar() != '\n') && c_tmp != EOF);
                    printf("\n\t请输入正确价格！\n\t价格:");

                };

                printf("\n\t出版社:");
                scanf("%s", publish);

                printf("\t═══════════════════════════════════════\n");
                printf("\t书架号\t\t类别\t\t\n");
                while (row = mysql_fetch_row(result)) {
                    printf("\t%s\t\t%s\n", row[0], row[1]);
                }

                printf("\n\t图书书架号:");
                while (1) {
                    if (scanf("%d", &locationUser) == 1) {
                        //printf("\n%d\n", locationUser);
                        break;
                    }
                    char c_tmp;
                    while ((c_tmp = getchar() != '\n') && c_tmp != EOF);
                    printf("\n\t请输入正确书架号！\n\t书架号:");

                };


                printf("\n\t图书简介:");
                scanf("%s", info);

                sprintf(price, "%.2f", priceUser);
                itoa(locationUser, location, 10);

                strcat(query, "insert into book values ('");
                strcat(query, "0");
                strcat(query, "','");
                strcat(query, bookname);
                strcat(query, "','");
                strcat(query, isbn);
                strcat(query, "','");
                strcat(query, writer);
                strcat(query, "','");
                strcat(query, price);
                strcat(query, "','");
                strcat(query, publish);
                strcat(query, "',");
                strcat(query, location);
                strcat(query, ", ");
                strcat(query, " 0 ,'");
                strcat(query, info);
                strcat(query, "');");

                //printf("\n\t%s", query);

                printf("\n\t请输入要添加多少本: ");
                if (scanf("%d", &nBook) != 1) {
                    printf("\n\t输入数据无效：共添加1本");
                }

                int flag = 1;
                for (int k = 0; k < nBook; ++k) {
                    if (mysql_query(&mysql, query) != 0) {
                        mysql_query(&mysql, "rollback;");
                    } else {
                        //printf("\n\t 插入成功!\n");
                        mysql_query(&mysql, "commit;");
                    }
                }
                if (flag) {
                    printf("\n\t 插入成功!\n");
                } else {
                    printf("\n\t 插入记录失败!\n%s\n", mysql_error(&mysql));
                }

                printf("\n\t 是否继续插入？\t\t是【Y】/否【N】 :");
                while (1) {
                    j = getchar();
                    if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                }
                if (j == 'N' || j == 'n')break;
                else continue;
            }

        }

        mysql_close(&mysql); //释放连接

    }

}

//修改图书信息
int Modifybook() {
    mysql_init(&mysql);
    if (!(mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0))) {
        printf("【震惊！保存图书的数据库连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到管理员的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from bookshelf;")) {
            printf("\n\t【震惊！保存书架信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            char j;
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            while (1) {
                //有记录数据时，才显示记录数据
                //system("cls");
                printf("\n\t\t\t════════════════════════════ \n");
                printf("\n\t\t\t        修 改 图 书 信 息                  \n");
                printf("\n\t\t\t════════════════════════════ \n");

                memset(query, 0, sizeof(char) * MAXLENGTH);

                while (1) {
                    printf("\n\t\t\t图书isbn:");
                    scanf("%s", isbn);

                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
					FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
					GROUP BY isbn) finalBook WHERE isbn LIKE '%");
                    strcat(query, isbn);
                    strcat(query, "%';");
                    //printf("%s", query);
                    if (mysql_query(&mysql, query)) {
                        printf("\n\t【震惊！保存书架信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                        getchar();
                        mysql_close(&mysql);
                        return 1;
                    } else {
                        result = mysql_store_result(&mysql);//获得结果集
                        rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                        if (rowcount == 0) {
                            printf("输入的isbn号不存在哦；");
                            continue;
                        } else {
                            printf("\t\t\t════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n");
                            //printf("\t\t\t║    书号 \t\t  书名  \t\t      图书号\t\t作者  \t  价钱  \t出版社  \t 书架号  ║");
                            printf("\t\t\t ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s", "书号", "书名", "图书号", "作者",
                                   "价格", "出版社", "书架号");
                            while (row = mysql_fetch_row(result)) {
                                printf("\n\t\t\t ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s ", row[0], row[1],
                                       row[2], row[3], row[4], row[5], row[6]);
                                printf("\n\t图书简介:%s", row[8]);
                                printf("\n\t图书类别:%s\t\t馆藏总数:%s\t\t馆藏现存:%s", row[10], row[11], row[12]);
                                printf("\n-----------------------------------------------------------------------------------------------------------\n");
                            }
                        }
                        break;
                    }
                }

                system("cls");
                printf("\t ════════════════════════════ \n");
                printf("\t         修 改 图 书 信 息                  \n");
                printf("\t ════════════════════════════ \n");
                memset(query, 0, sizeof(char) * MAXLENGTH);
                strcat(query, "SELECT * FROM(SELECT*, COUNT(*) 馆藏总数,SUM(IF(borrowed=0,1,0)) 馆藏现存\
					FROM(SELECT * FROM book b INNER JOIN bookshelf bs ON b.`location`=bs.`bookshelf` ORDER BY borrowed ASC) newBook\
					GROUP BY isbn) finalBook WHERE isbn LIKE '%");
                strcat(query, isbn);
                strcat(query, "%';");
                //printf("%s", query);
                if (mysql_query(&mysql, query)) {
                    printf("\n\t【震惊！保存书架信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                    getchar();
                    mysql_close(&mysql);
                    return 1;
                } else {
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("输入的isbn号不存在哦；"); }
                    else {
                        printf("\t\t\t════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n");
                        //printf("\t\t\t║    书号 \t\t  书名  \t\t      图书号\t\t作者  \t  价钱  \t出版社  \t 书架号  ║");
                        printf("\t\t\t ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s", "书号", "书名", "图书号", "作者", "价格",
                               "出版社", "书架号");
                        while (row = mysql_fetch_row(result)) {
                            printf("\n\t\t\t ☆ %-8s\t%-30s\t%-20s   %-10s%-10s%-20s\t   %-5s ", row[0], row[1], row[2],
                                   row[3], row[4], row[5], row[6]);
                            printf("\n\t图书简介:%s", row[8]);
                            printf("\n\t图书类别:%s\t\t馆藏总数:%s\t\t馆藏现存:%s", row[10], row[11], row[12]);
                            printf("\n-----------------------------------------------------------------------------------------------------------\n");
                        }
                    }
                }

                mysql_query(&mysql, query);
                result = mysql_store_result(&mysql);//获得结果集
                rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                row = mysql_fetch_row(result);

                while (1) {
                    printf("\t是否更改书名？\t\t\t是【Y】/否【N】：");
                    while (1) {
                        j = getchar();
                        if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                    }
                    if (j == 'N' || j == 'n') {
                        strcpy(bookname, row[1]);
                        break;
                    } else {
                        printf("\t书名:");
                        scanf("%s", bookname);
                        break;
                    };
                }

                while (1) {
                    printf("\t是否更改作者？\t\t\t是【Y】/否【N】：");
                    while (1) {
                        j = getchar();
                        if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                    }
                    if (j == 'N' || j == 'n') {
                        strcpy(writer, row[3]);
                        break;
                    } else {
                        printf("\n\t作者:");
                        scanf("%s", writer);
                        break;
                    };
                }

                while (1) {
                    printf("\t是否更改价格？\t\t\t是【Y】/否【N】：");
                    while (1) {
                        j = getchar();
                        if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                    }
                    if (j == 'N' || j == 'n') {
                        strcpy(price, row[4]);
                        break;
                    } else {
                        printf("\n\t价格:");
                        while (1) {
                            if (scanf("%lf", &priceUser) == 1) {
                                //printf("\n%lf\n", priceUser);
                                sprintf(price, "%.2f", priceUser);
                                break;
                            } else {
                                char c_tmp;
                                while ((c_tmp = getchar() != '\n') && c_tmp != EOF);
                                printf("\n\t请输入正确价格！\n\t价格:");
                            }

                        };
                        break;
                    };
                }

                while (1) {
                    printf("\t是否更改出版社？\t\t是【Y】/否【N】：");
                    while (1) {
                        j = getchar();
                        if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                    }
                    if (j == 'N' || j == 'n') {
                        strcpy(publish, row[5]);
                        break;
                    } else {
                        printf("\t出版社:");
                        scanf("%s", publish);
                        break;
                    };
                }

                while (1) {
                    printf("\t是否更改图书简介？\t\t是【Y】/否【N】：");
                    while (1) {
                        j = getchar();
                        if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                    }
                    if (j == 'N' || j == 'n') {
                        strcpy(info, row[8]);
                        break;
                    } else {
                        printf("\n\t图书简介:");
                        scanf("%s", info);
                        break;
                    };
                }


                if (mysql_query(&mysql, "select * from bookshelf;")) {
                    printf("\n\t【震惊！保存书架信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                    getchar();
                    mysql_query(&mysql, "rollback;");
                    mysql_close(&mysql);
                    return 1;
                } else {

                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                }

                while (1) {
                    printf("\t是否更改书架号？\t\t是【Y】/否【N】：");
                    while (1) {
                        j = getchar();
                        if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                    }
                    if (j == 'N' || j == 'n') {
                        strcpy(location, row[6]);
                        break;
                    } else {

                        printf("\t═══════════════════════════════════════\n");
                        printf("\t书架号\t\t类别\t\t\n");
                        while (row = mysql_fetch_row(result)) {
                            printf("\t%s\t\t%s\n", row[0], row[1]);
                        }

                        printf("\n\t图书书架号:");
                        while (1) {
                            if (scanf("%d", &locationUser) == 1) {
                                //printf("\n%d\n", locationUser);
                                itoa(locationUser, location, 10);
                                break;
                            }
                            char c_tmp;
                            while ((c_tmp = getchar() != '\n') && c_tmp != EOF);
                            printf("\n\t请输入正确书架号！\n\t书架号:");
                        };
                        break;
                    };
                }

                memset(query, 0, sizeof(char) * MAXLENGTH);
                strcat(query, "update book set book_name='");
                strcat(query, bookname);
                strcat(query, "',isbn='");
                strcat(query, isbn);
                strcat(query, "',writer='");
                strcat(query, writer);
                strcat(query, "',price=");
                strcat(query, price);
                strcat(query, " , publish='");
                strcat(query, publish);
                strcat(query, "',location=");
                strcat(query, location);
                strcat(query, ",info= '");
                strcat(query, info);
                strcat(query, "' ");
                strcat(query, "where isbn='");
                strcat(query, isbn);
                strcat(query, "';");
                printf("%s", query);
                if (mysql_query(&mysql, query) != 0) {
                    printf("\t 修改失败!\n%s\n", mysql_error(&mysql));
                    mysql_query(&mysql, "rollback;");
                } else {
                    printf("\n\n\t 修改成功!\n");
                    mysql_query(&mysql, "commit;");
                }
                printf("\t是否继续修改？\t\t是【Y】/否【N】：");
                while (1) {
                    j = getchar();
                    if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                }
                if (j == 'N' || j == 'n')break;
                else {
                    mysql_close(&mysql); //释放连接
                    return 2; //继续调用的信号
                };
            }

        }

        mysql_close(&mysql); //释放连接
    }
    return 0;
}


//删除图书
int Deletebook() {
    mysql_init(&mysql);
    if (!(mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0))) {
        printf("【震惊！保存图书的数据库连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到管理员的表
        mysql_query(&mysql, "set names 'gbk'");
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from bookshelf;")) {
            printf("\n\t【震惊！保存书架信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            char j;
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            while (1) {
                //有记录数据时，才显示记录数据
                printf("\n\t\t\t请输入要删除的图书ISBN：");
                memset(query, 0, sizeof(char) * MAXLENGTH);
                scanf("%s", isbn);
                strcat(query, "delete from book where isbn='");
                strcat(query, isbn);
                strcat(query, "';");
                //printf("%s", query);
                if (mysql_query(&mysql, query) != 0) {
                    printf("\n\t\t\t 删除失败!\n%s\n", mysql_error(&mysql));
                    mysql_query(&mysql, "rollback;");
                } else {
                    printf("\n\t\t\t 删除成功!\n");
                    mysql_query(&mysql, "commit;");
                }
                printf("\n\t\t\t是否继续删除？\t\t是【Y】/否【N】:");
                while (1) {
                    j = getchar();
                    if (j == 'Y' || j == 'y' || j == 'N' || j == 'n') break;
                }
                if (j == 'N' || j == 'n')break;
                else {
                    mysql_close(&mysql); //释放连接
                    return 2; //继续调用的信号
                };
            }

        }

        mysql_close(&mysql); //释放连接
    }
}

//操作图书信息
int Operatebook() {
    //查询图书信息
    int checkbook();
    //增添图书
    int Addbook();
    //修改图书
    int Modifybook();
    //删除图书
    int Deletebook();
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到图书的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from book")) {
            printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行

            while (1) {

                system("cls");
                bookinforMenu1();
                scanf("%s", type1);
                if (strcmp(type1, "1") == 0) {
                    system("cls");
                    Checkbook(0);
                } else if (strcmp(type1, "2") == 0) {
                    DMbook();
                    printf("\n\t\t\t");
                    getchar();
                    continue;

                } else if (strcmp(type1, "3") == 0) {
                    Addbook();
                    printf("\n\n\t");
                    getchar();
                    continue;
                } else if (strcmp(type1, "0") == 0) {
                    //mysql_free_result(result);
                    mysql_query(&mysql, "commit;");
                    mysql_close(&mysql); //释放连接
                    break;
                } else {
                    continue;
                }
            }
            return 0;
        }
    }

}

//删除/修改图书信息
int DMbook() {
    //删除图书
    int Deletebook();
    // 修改图书信息
    int Modifybook();

    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到图书的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from book")) {
            printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            printf("\n\n\t\t\t将跳转到图书查询菜单，并选择下一步的操作\n\t\t\t");
            getchar();
            while (Checkbook(2) == 3) {};
        }
    }
}


//-------------------链表5
//链表
struct Node5 {
    char row0[MAXLENGTH];
    char row1[MAXLENGTH];
    char row2[MAXLENGTH];
    char row3[MAXLENGTH];
    char row4[MAXLENGTH];
    struct Node5 *next;
};
struct Node5 *head5 = NULL;
struct Node5 *end5 = NULL;

int AddList5(char *row0, char *row1, char *row2, char *row3, char *row4) {    //创建一个节点
    struct Node5 *temp = (struct Node5 *) malloc(sizeof(struct Node5));        //此处注意强制类型转换

    //节点数据进行赋值
    strcpy(temp->row0, row0);
    strcpy(temp->row1, row1);
    strcpy(temp->row2, row2);
    strcpy(temp->row3, row3);
    strcpy(temp->row4, row4);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head5) {

        head5 = temp;
        //	end=temp;
    } else {
        end5->next = temp;
        //	end=temp;			//尾结点应该始终指向最后一个
    }
    end5 = temp;            //尾结点应该始终指向最后一个
}

int ScanList5() {
    struct Node5 *temp = head5;        //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("%-10s\t\t%-30s\t\t%-30s\t\t%-30s\t\t%-10s\n", temp->row0, temp->row1, temp->row2, temp->row3,
               temp->row4);
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
    }

}

int FreeList5() {
    //一个一个NULL
    struct Node *temp = head5;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node5 *pt = temp;
        temp = pt->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head5 = NULL;
    end5 = NULL;
}

// 查看借书信息
int borrowinfo() {
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        //接着看是否能连接到借阅人的表
        mysql_query(&mysql, "set names 'gbk'");
        if (mysql_query(&mysql, "select * from borrowed")) {
            printf("\n\t保存借阅人信息的表格连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            while (1) {
                system("cls");
                bookinforMenu3();
                scanf("%s", type2);
                if (strcmp(type2, "1") == 0) {
                    printf("\n\n════════════════════════════════════════════════════════════════════════════\n");
                    printf("请输入借阅人姓名：");
                    scanf("%s", readername);
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT bo.book_id,book_name,borrow_times,return_real,IF(return_plan>return_real,'未超期','超期') FROM returned bo INNER JOIN book b\
				ON bo.`book_id`=b.`book_id` INNER JOIN reader r ON bo.`reader_id`= r.reader_id WHERE r.reader_name LIKE '%");
                    strcat(query, readername);
                    strcat(query, "%';");
                    //printf("%s", query);
                    if (mysql_query(&mysql, query)) {
                        printf("\n\t【错误：%s】\n请按任意键结束这次不愉快的体验吧，对不起了", mysql_error(&mysql));
                        getchar();
                        mysql_close(&mysql);
                        return 1;
                    }
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("这个人还没有借阅记录！"); }
                    else {
                        printf("══════════════════════════════════════════════════════════════════════════\n");
                        printf("%-10s\t\t%-30s\t\t%-30s\t\t%-30s\t\t%-10s\n", "书号", "书名", "借书时间", "归还时间", "是否超期");
                        while (row = mysql_fetch_row(result)) {
                            printf("%-10s\t\t%-30s\t\t%-30s\t\t%-30s\t\t%-10s\n", row[0], row[1], row[2], row[3],
                                   row[4]);
                        }
                    }
                    getchar();
                    continue;
                } else if (strcmp(type2, "2") == 0) {
                    printf("\n\n\t\t\t将跳转到图书查询菜单，并选择下一步的操作\n\t\t\t");
                    getchar();
                    while (Checkbook(3) == 3) {};
                    printf("\n\n════════════════════════════════════════════════════════════════════════════\n");
                    printf("请输入图书isbn：");
                    scanf("%s", bookname1);
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT r.reader_id,r.reader_name,borrow_times,return_plan,IF(return_plan>NOW(),'未还：未超期','未还：超期') FROM reader r INNER JOIN borrowed bo ON r.`reader_id`=bo.`reader_id`\
						INNER JOIN book b ON b.`book_id`=bo.`book_id` WHERE isbn = '");
                    strcat(query, bookname1);
                    strcat(query, "' AND valid = 1 UNION\
						SELECT r.reader_id,r.reader_name,borrow_times,return_plan,IF(return_plan>return_real,'已还：未超期','已还：超期') FROM reader r INNER JOIN returned bo ON r.`reader_id`=bo.`reader_id`\
						INNER JOIN book b ON b.`book_id`=bo.`book_id` WHERE isbn = '");
                    strcat(query, bookname1);
                    strcat(query, "';");
                    //printf("%s", query);
                    if (mysql_query(&mysql, query)) {
                        printf("\n\t【错误：%s】\n请按任意键结束这次不愉快的体验吧，对不起了", mysql_error(&mysql));
                        getchar();
                        mysql_close(&mysql);
                        return 1;
                    }
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("这本书没有人借阅！"); }
                    else {
                        printf("══════════════════════════════════════════════════════════════════════════\n");
                        printf("%-10s\t\t%-30s\t\t%-30s\t\t%-30s\t\t%-10s\n", "用户名", "姓名", "借书时间", "归还时间", "是否超期");
                        while (row = mysql_fetch_row(result)) {
                            AddList5(row[0], row[1], row[2], row[3], row[4]);
                            //printf("\t%s\t\t%s\t\t%s\t\t\t%s\t\t%s\n", row[0], row[1], row[2], row[3], row[4]);
                        }
                        ScanList5();
                        FreeList5();
                    }
                    printf("\n\n\t");
                    getchar();
                    continue;
                } else if (strcmp(type2, "0") == 0) {
                    //mysql_free_result(result);
                    mysql_query(&mysql, "commit;");
                    mysql_close(&mysql); //释放连接
                    break;
                } else {
                    continue;
                }
            }
        }
    }
}


//-------------------链表6
struct Node6 {
    char row0[MAXLENGTH];
    char row1[MAXLENGTH];
    struct Node6 *next;
};
struct Node6 *head6 = NULL;
struct Node6 *end6 = NULL;

int AddList6(char *row0, char *row1) {    //创建一个节点
    struct Node6 *temp = (struct Node6 *) malloc(sizeof(struct Node6));        //此处注意强制类型转换

    //节点数据进行赋值
    strcpy(temp->row0, row0);
    strcpy(temp->row1, row1);
    temp->next = NULL;

    //连接分两种情况1.一个节点都没有2.已经有节点了，添加到尾巴上
    if (NULL == head6) {

        head6 = temp;
        //	end=temp;
    } else {
        end6->next = temp;
        //	end=temp;			//尾结点应该始终指向最后一个
    }
    end6 = temp;            //尾结点应该始终指向最后一个
}

int ScanList6() {
    struct Node6 *temp = head6;        //定义一个临时变量来指向头
    while (temp != NULL) {
        printf("\t%-30s\t%-30s\n", temp->row0, temp->row1);
        temp = temp->next;        //temp指向下一个的地址 即实现++操作
    }

}

int FreeList6() {
    //一个一个NULL
    struct Node *temp = head6;        //定义一个临时变量来指向头
    while (temp != NULL) {
        //	printf("%d\n",temp->a);
        struct Node6 *pt = temp;
        temp = pt->next;        //temp指向下一个的地址 即实现++操作
        free(pt);                    //释放当前
    }
    //头尾清空	不然下次的头就接着0x10
    head6 = NULL;
    end6 = NULL;
}


//统计
int countlist() {
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到图书的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from borrowed")) {
            printf("\n\t【震惊！保存借阅信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            while (1) {
                system("cls");
                countMenu();
                scanf("%s", type);
                //借书总数
                //memset(query, 0, sizeof(char) * MAXLENGTH);
                //strcat(query, "SELECT COUNT(*) FROM returned re INNER JOIN reader r\
				//		ON re.`reader_id`=r.`reader_id`;");
                //mysql_query(&mysql, query);
                //result = mysql_store_result(&mysql);//获得结果集
                //rowcount = (int)mysql_num_rows(result);//获得结果集有多少行
                //row = mysql_fetch_row(result);
                //sum1 =atoi(row[0]);
                if (strcmp(type, "1") == 0) {

                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*) FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_department`='1'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("没有记录！！！！！！"); }
                    else {
                        printf("\t══════════════════════════════════════════════════════════════════════════\n");
                        printf("\t学院名称\t\t借阅人数量\t\t借阅书籍数量\n");
                        while (row = mysql_fetch_row(result)) {
                            printf("\t计算机学院\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*) FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_department`='2'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("出现了一个预料之外的错误！！！！！！"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t理学院\t\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*) FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_department`='3'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("没有记录！！！！！！"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t文法学院\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*) FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_department`='4'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("出现了一个预料之外的错误！！！！！！"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t生命科学学院\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    printf("\n\n\t");
                    getchar();
                    continue;
                } else if (strcmp(type, "2") == 0) {

                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*) FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_sex`='1'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("出现了一个预料之外的错误！！！！！！"); }
                    else {
                        printf("\t══════════════════════════════════════════════════════════════════════════\n");
                        printf("\t性别\t\t借阅人数量\t\t借阅书籍数量\n");
                        while (row = mysql_fetch_row(result)) {
                            printf("\t男性\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*) FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_sex`='2'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("没有记录！！！！！！"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t女性\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    printf("\n\n\t");
                    getchar();
                    continue;

                } else if (strcmp(type, "3") == 0) {
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='1'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("出现了一个预料之外的错误！！！！！！"); }
                    else {
                        printf("\t══════════════════════════════════════════════════════════════════════════\n");
                        printf("\t年级\t\t借阅人数量\t\t借阅书籍数量\t\t\n");
                        while (row = mysql_fetch_row(result)) {
                            printf("\t大一\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }

                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='2'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("请检查输入的编号是否正确。"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t大二\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='3'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("出现了一个预料之外的错误！！！！！！"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t大三\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='4'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("出现了一个预料之外的错误！！！！！！"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t大四\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='5'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("请检查输入的编号是否正确。"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t研究生\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='6'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("请检查输入的编号是否正确。"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t博士生\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT COUNT(DISTINCT r.reader_id),COUNT(*)  FROM returned re INNER JOIN reader r\
						ON re.`reader_id`=r.`reader_id` WHERE r.`reader_grade`='7'; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("请检查输入的编号是否正确。"); }
                    else {
                        while (row = mysql_fetch_row(result)) {
                            printf("\t教师\t\t%s\t\t\t%s\n", row[0], row[1]);
                        }
                    }
                    printf("\n\n\t");
                    getchar();
                    continue;
                } else if (strcmp(type, "4") == 0) {
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "SELECT  b.`book_name`,COUNT(*)FROM borrowed   bo INNER JOIN book b ON b.`book_id`=bo.`book_id`\
						GROUP BY b.`isbn` ORDER BY  COUNT(*) DESC LIMIT 0, 3; ");
                    //printf("%s", query);
                    mysql_query(&mysql, query);
                    //eymysql_free_result(result);
                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) { printf("没有记录"); }
                    else {
                        printf("\n\n\t══════════════════════════════════════════════════════════════════════════\n");
                        printf("\t最受欢迎书籍：\n\n\n");
                        printf("\t%-30s\t%-30s\n\n", "书名", "借阅次数");
                        while (row = mysql_fetch_row(result)) {
                            AddList6(row[0], row[1]);
                        }
                        ScanList6();
                        FreeList6();
                    }
                    printf("\n\n\t");
                    getchar();
                    continue;
                } else if (strcmp(type, "0") == 0) {
                    //mysql_free_result(result);
                    mysql_query(&mysql, "commit;");
                    mysql_close(&mysql); //释放连接
                    break;
                } else {
                    continue;
                }
            }
            return 0;
        }
    }
}

//罚金
int checkfines() {   //首先连接数据库
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到图书的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        //mysql_query(&mysql, "rollback;");
        if (mysql_query(&mysql, "select * from borrowed")) {
            printf("\n\t【震惊！保存借阅信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
            getchar();
            mysql_query(&mysql, "rollback;");
            mysql_close(&mysql);
            return 1;
        } else {
            result = mysql_store_result(&mysql);//获得结果集
            rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
            while (1) {
                system("cls");
                checkfinesMenu();
                scanf("%s", fines);
                if (strcmp(fines, "1") == 0) {
                    printf("\t════════════════════════════\n\t");
                    printf("请输入学生用户名：");
                    scanf("%s", reader_id1);
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "select * from reader where reader_id ='");
                    strcat(query, reader_id1);
                    strcat(query, "';");
                    // printf("%s\n", query);
                    mysql_query(&mysql, query);

                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) {
                        printf("\n\t输入的用户名不存在。\n\t");
                        getchar();
                        continue;
                    } else {
                        row = mysql_fetch_row(result);
                        //没有罚金时
                        if (strcmp(row[10], "0") == 0) {
                            printf("\n\n\t你没有罚金未结清啊，尽情享受阅读吧！\n");
                            getchar();
                            mysql_free_result(result);
                            mysql_query(&mysql, "commit;");
                            mysql_close(&mysql);
                            return 0;
                        }

                        //修改个人罚金为0
                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_penalty=0 where reader_id = ");
                        strcat(query, "'");
                        strcat(query, reader_id1);
                        strcat(query, "';");
                        mysql_query(&mysql, query);

                        if (1) {
                            printf("\n\n\t【罚金已经结清！！!】\n");
                            Sleep(800);
                            mysql_free_result(result);    //释放结果集
                            mysql_query(&mysql, "commit;");
                            mysql_close(&mysql);          //释放连接
                            return 0;
                        }

                    }
                    continue;
                }
                if (strcmp(fines, "2") == 0) {
                    printf("\t════════════════════════════\n\t");
                    printf("请输入学生用户名：");
                    scanf("%s", reader_id1);
                    memset(query, 0, sizeof(char) * MAXLENGTH);
                    strcat(query, "select * from reader where reader_id ='");
                    strcat(query, reader_id1);
                    strcat(query, "';");
                    // printf("%s\n", query);
                    mysql_query(&mysql, query);

                    result = mysql_store_result(&mysql);//获得结果集
                    rowcount = (int) mysql_num_rows(result);//获得结果集有多少行
                    if (rowcount == 0) {
                        printf("\n\t输入的用户名不存在。\n\t");
                        getchar();
                        continue;
                    } else {
                        row = mysql_fetch_row(result);

                        //修改个人罚金为0
                        memset(query, 0, sizeof(char) * MAXLENGTH);
                        strcat(query, "update reader set reader_penalty=1 where reader_id = ");
                        strcat(query, "'");
                        strcat(query, reader_id1);
                        strcat(query, "';");
                        mysql_query(&mysql, query);

                        if (1) {
                            printf("\n\n\t【罚金已经增加！！!】\n\t");
                            Sleep(800);
                            mysql_free_result(result);    //释放结果集
                            mysql_query(&mysql, "commit;");
                            mysql_close(&mysql);          //释放连接
                            return 0;
                        }

                    }
                    continue;
                }
                if (strcmp(fines, "0") == 0) {
                    //mysql_free_result(result);
                    mysql_query(&mysql, "commit;");
                    mysql_close(&mysql); //释放连接
                    break;
                } else {
                    continue;
                }
            }
        }
    }
}

//备份
int Backup() {
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, IP, USERNAME, PASSWORD, "library", 3306, NULL, 0)) {
        printf("保存书籍信息的数据库连不上了\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_close(&mysql);
        return 1;
    } else {
        mysql_query(&mysql, "set names 'gbk'");
        //接着看是否能连接到图书的表
        mysql_query(&mysql, "set autocommit=0;");
        //mysql_query(&mysql, "commit;");
        int i;
        while (1) {
            system("cls");
            char road[MAXLENGTH];
            char a[MAXLENGTH] = "mysqldump -u";
            char a2[MAXLENGTH] = "mysql -u";
            strcat(a, USERNAME);
            strcat(a, " -p");
            strcat(a, PASSWORD);
            strcat(a2, USERNAME);
            strcat(a2, " -p");
            strcat(a2, PASSWORD);
            printf("\n\n\n");
            printf("\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
            printf("\t║\t\t☆ 1.备份全部数据\t☆ 2.还原全部数据\t\t\t║\n");
            printf("\t║\t\t☆ 3.备份图书信息\t☆ 4.还原图书信息\t\t\t║\n");
            printf("\t║\t\t☆ 5.备份账户信息\t☆ 6.还原账户信息\t\t\t║\n");
            printf("\t║\t\t☆ 7.备份借阅记录\t☆ 8.还原借阅记录\t\t\t║\n");
            printf("\t║\t\t☆ 9.备份校园卡信息\t☆ 10.还原校园卡信息\t\t\t║\n");
            printf("\t║\t\t☆ 11.修改默认备份路径\t☆ 0.不做任何此操作\t\t\t║\n");
            printf("\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
            printf("\t请选择(1-11）:");
            scanf("%d", &i);
            if (mysql_query(&mysql, "select * from backuproad")) {
                printf("\n\t【震惊！保存备份路径信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                getchar();
                mysql_query(&mysql, "rollback;");
                mysql_close(&mysql);
                return 1;
            } else {
                /*数据库连接成功*/
                result = mysql_store_result(&mysql); //获得结果集

                while ((row = mysql_fetch_row(result))) {   //取出结果集中记录
                    strcpy(road, row[0]);
                }

                if (i == 1) {
                    if (mysql_query(&mysql, "select * from book;")) {
                        printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                        getchar();
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    }
                    strcat(a, " library > ");
                    strcat(a, road);
                    strcat(a, "Backup1.sql");
                    system(a);

                    printf("\n\t备份成功！\n\t");
                    getchar();
                    break;
                } else if (i == 2) {
                    mysql_close(&mysql);
                    strcat(a2, " library < ");
                    strcat(a2, road);
                    strcat(a2, "Backup1.sql");
                    //printf(a2);
                    system(a2);
                    printf("\n\t还原成功！\n\t");
                    getchar();
                    break;
                } else if (i == 3) {
                    if (mysql_query(&mysql, "select * from book;")) {
                        printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                        getchar();
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    }
                    strcat(a, " library book bookshelf > ");
                    strcat(a, road);
                    strcat(a, "Backup2.sql");
                    system(a);
                    printf("\n\t备份成功！\n\t");
                    getchar();
                    break;
                } else if (i == 4) {

                    mysql_close(&mysql);
                    strcat(a2, " library < ");
                    strcat(a2, road);
                    strcat(a2, "Backup2.sql");
                    system(a2);
                    printf("\n\t还原成功！\n\t");
                    getchar();
                    break;
                } else if (i == 5) {
                    if (mysql_query(&mysql, "select * from book;")) {
                        printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                        getchar();
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    }
                    strcat(a, " library admin reader > ");
                    strcat(a, road);
                    strcat(a, "Backup3.sql");
                    system(a);
                    printf("\n\t备份成功！\n\t");
                    getchar();
                    break;
                } else if (i == 6) {
                    mysql_close(&mysql);
                    strcat(a2, " library < ");
                    strcat(a2, road);
                    strcat(a2, "Backup3.sql");
                    system(a2);
                    printf("\n\t还原成功！\n\t");
                    getchar();
                    break;
                } else if (i == 7) {
                    if (mysql_query(&mysql, "select * from book;")) {
                        printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                        getchar();
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    }
                    strcat(a, " library returned borrowed > ");
                    strcat(a, road);
                    strcat(a, "Backup4.sql");
                    system(a);
                    printf("\n\t备份成功！\n\t");
                    getchar();
                    break;
                } else if (i == 8) {

                    mysql_close(&mysql);
                    strcat(a2, " library < ");
                    strcat(a2, road);
                    strcat(a2, "Backup4.sql");
                    system(a2);
                    printf("\n\t还原成功！\n\t");
                    getchar();
                    break;
                } else if (i == 9) {
                    if (mysql_query(&mysql, "select * from book;")) {
                        printf("\n\t【震惊！保存图书信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
                        getchar();
                        mysql_query(&mysql, "rollback;");
                        mysql_close(&mysql);
                        return 1;
                    }
                    strcat(a, " library schoolcard > ");
                    strcat(a, road);
                    strcat(a, "Backup5.sql");
                    system(a);
                    printf("\n\t备份成功！\n\t");
                    getchar();
                    break;
                } else if (i == 10) {

                    mysql_close(&mysql);
                    strcat(a2, " library < ");
                    strcat(a2, road);
                    strcat(a2, "Backup5.sql");
                    system(a2);
                    printf("\n\t还原成功！\n\t");
                    getchar();
                    break;
                } else if (i == 11) {
                    SetBackupRoad();
                    mysql_close(&mysql);

                    getchar();
                    break;
                } else if (i == 0) {
                    break;
                } else continue;
            }


        }
    }
}

//备份路径设置
void SetBackupRoad() {
    char Pan[MAXLENGTH];
    char folder[MAXLENGTH];
    char r[MAXLENGTH] = " ";
    char s[MAXLENGTH] = " ";
    mysql_query(&mysql, "set names 'gbk'");
    if (mysql_query(&mysql, "select * from backuproad")) {
        printf("\n\t【震惊！保存备份路径信息的表格连不上了，快快给 lib_it@gmail.com 发封邮件让他们来修修】\n请按任意键结束这次不愉快的体验吧，对不起了");
        getchar();
        mysql_query(&mysql, "rollback;");
        mysql_close(&mysql);
        return 1;
    } else {
        /*数据库连接成功*/
        result = mysql_store_result(&mysql); //获得结果集

        while ((row = mysql_fetch_row(result))) {   //取出结果集中记录
            printf("\n\t\t默认备份路径：%s", row[0]);
        }
    }
    printf("\n\n\t\t☆★修改路径可能导致还原时找不到备份文件哦★☆");
    printf("\n\t\t\t☆★！！修改请谨慎！！★☆");
    printf("\n\n\t\t是否修改默认路径：(是【Y】/否【N】)");
    while (1) {
        scanf("%s", ch);
        if (strcmp(ch, "Y") == 0 || strcmp(ch, "y") == 0 || strcmp(ch, "N") == 0 || strcmp(ch, "n") == 0) break;
        else {
            printf("\n\t\t修改请输入【Y】/不修改请输入【N】\n");
        }
    }
    if (strcmp(ch, "Y") == 0 || strcmp(ch, "y") == 0) {
        while (1) {
            printf("\n\t\t请设置备份磁盘（C/D/E):");
            scanf("%s", Pan);
            if (strcmp(Pan, "C") == 0 || strcmp(Pan, "c") == 0 || strcmp(Pan, "D") == 0 || strcmp(Pan, "d") == 0 ||
                strcmp(Pan, "E") == 0 || strcmp(Pan, "e") == 0)
                break;
            else continue;
        }

        printf("\n\t\t是否设置文件夹：(是【Y】/否【N】)");

        while (1) {
            scanf("%s", ch);
            if (strcmp(ch, "Y") == 0 || strcmp(ch, "y") == 0 || strcmp(ch, "N") == 0 || strcmp(ch, "n") == 0) break;
            else {
                printf("\n\t\t修改请输入【Y】/不修改请输入【N】\n");
            }
        }

        if (strcmp(ch, "Y") == 0 || strcmp(ch, "y") == 0) {
            printf("\n\t\t请输入文件夹名称:");
            scanf("%s", folder);

            char path[100] = {0};
            strcat(path, "mkdir ");
            strcat(path, Pan);
            strcat(path, ":\\");
            strcat(path, folder);
            system(path);

            strcat(r, Pan);
            strcat(r, ":/");
            strcat(r, folder);
            strcat(r, "/");
            strcat(s, "update backuproad set road='");
            strcat(s, r);
            strcat(s, "'");

            if (mysql_query(&mysql, s) != 0) {
                fprintf(stderr, "\n\t\t 不能修改记录!\n%s\n", mysql_error(&mysql));
                mysql_query(&mysql, "rollback;");
            } else {
                printf("\n\t\t 修改成功!\n");
                mysql_query(&mysql, "commit;");
            }
        } else {
            strcat(r, Pan);
            strcat(r, ":/");
            strcat(s, "update backuproad set road='");
            strcat(s, r);
            strcat(s, "';");
            if (mysql_query(&mysql, s) != 0) {
                fprintf(stderr, "\n\t\t 修改失败!\n%s\n", mysql_error(&mysql));
                mysql_query(&mysql, "rollback;");
            } else {
                printf("\n\t\t 修改成功!\n");
                mysql_query(&mysql, "commit;");
            }
        }
    }


}












