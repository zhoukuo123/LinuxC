#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

void my_err(const char *str, int line) {
    fprintf(stderr, "line:%d ", line);
    perror(str);
    exit(1);
}

MYSQL accept_mysql() {
    MYSQL mysql;

    if (mysql_init(&mysql) == NULL) {
        my_err("mysql_init", __LINE__);
    }

    // 初始化数据库
    if (mysql_library_init(0, NULL, NULL) != 0) {
        my_err("mysql_library_init", __LINE__);
    }

    // 连接数据库
    if (mysql_real_connect(&mysql, "127.0.0.1", "root", "zhoukuo", "test", 0, NULL, 0) == NULL) {
        my_err("mysql_real_connect", __LINE__);
    }

    // 设置中文字符集
    if (mysql_set_character_set(&mysql, "uft8") < 0) {
        my_err("mysql_set_character_set", __LINE__);
    }

    printf("连接mysql数据库成功!\n");
    return mysql;
}

int use_mysql(const char *string, MYSQL mysql1) {
    int i;
    int ret;
    unsigned int num_fields;
    MYSQL mysql = mysql1;
    MYSQL_RES *result = NULL;
    MYSQL_ROW row;
    MYSQL_FIELD *field;

    ret = mysql_query(&mysql, string);
    if (!ret) {
        result = mysql_store_result(&mysql);
        if(result) {
            num_fields = mysql_num_fields(result);
            while((field = mysql_fetch_field(result))) {
                printf("%-20s", field->name);
            }
            printf("\n");
            row = mysql_fetch_row(result);
            while(row) {
                for(i = 0; i < num_fields; i++) {
                    if(row[i]){
                        printf("%-20s", row[i]);
                    }
                }
                printf("\n");
                row = mysql_fetch_row(result);
            }
        }
        mysql_free_result(result);
    }
    else {
        printf("query fail\n");
        return -1;
    }
    return 0;
}

int close_mysql(MYSQL mysql) {
    mysql_close(&mysql);
    mysql_library_end();
    printf("end\n");
    return 0;
}

int main() {

    MYSQL mysql1 = accept_mysql();

    use_mysql("select * from course", mysql1);
    close_mysql(mysql1);
    return 0;


}
