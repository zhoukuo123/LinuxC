#include "../common/main.h"

int main() {

    //part0 设置窗口
    widget();

    //part1	初始化数据库
    mysql_init(&mysql);
    // progressBar(); //模拟初始化进度

    //第一部分：登陆+注册
    int identity = 0;
////////////////****** identity-- 要改成 1  *******/////////////////////////////////////////////////
    while (1) {
        system("clear");
        loginMenu();
        char input = getchar();
        // sleep(500);
        if (input == '1') {
            system("clear");
            if (readerLogin() != 0) {
                // sleep(800)
                continue;
            }
            identity = 1;
            break;
        } else if (input == '2') {
            system("clear");
            if (adminLogin() != 0) {
                continue;
            }
            identity = 2;
            break;
        } else if (input == '3') {
            system("clear");
            if (registerAccount() == 0) {
                continue;
            }
        } else if (input == '4') {
            system("clear");
            printf("感谢您的使用!");
            exit(0);
        } else {
            continue;
        }
    }


    // 功能界面选择
    // 读者菜单部分
    if (identity == 1) {
        while (1) {
            system("clear");
            readerMenu();
            char input = getchar();
            // 借书
            if (input == '1') {
                system("clear");
                printf("\n\n\t\t\t将跳转到图书查询菜单，并选择下一步的操作\n\t\t\t");
                getchar();
                if (checkbook(1) == 0) {
                    continue;
                } else {
                    borrowBooks(book_id_next, login_id);
                    continue;
                }
            } else if (input == '2') {  //查询图书信息并还书
                system("clear");
                if (checkBorrowed() == 0) {
                    continue;
                } else {
                    returnBooks(checkBorrowed_ret, login_id);
                    continue;
                }
            } else if (input == '3') { //查询个人信息并修改
                system("clear");
                if (checkInfo() == 1) {
                    getchar();
                    continue;
                } else {
                    updateInfo(updateNum, login_id);
                    continue;
                }
                continue;
            } else if (input == '4') { //申请续借
                system("clear");
                if (checkRenew() == 1) {
                    continue;
                } else {
                    renew(checkBorrowed_ret, login_id);
                    continue;
                }
            } else if (input == '5') { //交罚金
                system("clear");
                payDelayPenalty(login_id);
                continue;
            } else if (input == '0') { //退出
                system("clear");
                printf("感谢您的使用！");
                exit(0);
            } else {
                continue;
            }
        }
    } else { //管理员菜单
        while (1) {
            system("clear");
            adminMenu();
            char input = getchar();
            sleep(500);
            if (input == '1') {
                system("clear");
                SecPi();
                continue;
            } else if (input == '2') {
                system("clear");
                ModifyPi();
                continue;
            } else if (input == '3') {
                system("clear");
                Operatebook();
                continue;
            } else if (input == '4') {
                system("clear");
                borrowinfo();
                continue;
            } else if (input == '5') {
                system("clear");
                checkfines();
                continue;
            } else if (input == '6') {
                system("clear");
                countlist();
                continue;
            } else if (input == '0') {
                break;
            } else {
                continue;
            }
        }
    }

    return 0;
}
