#include "../include/menu.h"

//菜单1：登陆+注册选择
void loginMenu() {
    printf("\t\t\t\t\n");
    printf("\t\t\t\t                 .-~~~~~~~~~-._       _.-~~~~~~~~~-.\n");
    printf("\t\t\t\t             __.'              ~.   .~              `.__\n");
    printf("\t\t\t\t           .'//                  \\./                  \\`.\n");
    printf("\t\t\t\t         .'//                     |                     \\`.\n");
    printf("\t\t\t\t       .'// .-~""""""""~~~~-._            |              _-~~~~""""""""~-. \\`.\n");
    printf("\t\t\t\t     .'//.-\"                 \"`-. |  .-'                 ""-.\\`.\n");
    printf("\t\t\t\t   .'//______.============-..   \\ | /   ..-============.______\\`.\n");
    printf("\t\t\t\t .'______________________________\\|/______________________________`.\n");
    printf("\t\t\t\t\n");
    printf("\n\n\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t请选择您的身份\t\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 借阅人\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 管理员\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t3 - 注册\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t4 - 退出\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(1-4):");
}

//菜单2：借阅人菜单
void readerMenu() {
    printf("\n");
    printf("\t\t\t              ,----------------,              ,---------,\n");
    printf("\t\t\t         ,-----------------------,          ,\"        ,\"|\n");
    printf("\t\t\t       ,\"                      ,\"|        ,\"        ,\"  |\n");
    printf("\t\t\t      +-----------------------+  |      ,\"        ,\"    |\n");
    printf("\t\t\t      |  .-----------------.  |  |     +---------+      |\n");
    printf("\t\t\t      |  |                 |  |  |     | -==----'|      |\n");
    printf("\t\t\t      |  |  I LOVE DOS!    |  |  |     |         |      |\n");
    printf("\t\t\t      |  |  Bad command or |  |  |/----|`---=    |      |\n");
    printf("\t\t\t      |  |  C:\\>_          |  |  |   ,/|==== ooo |      ;\n");
    printf("\t\t\t      |  |                 |  |  |  // |(((( [33]|    ,\"\n");
    printf("\t\t\t      |  `-----------------'  |,\" .;\'| |((((     |  ,\"\n");
    printf("\t\t\t      +-----------------------+  ;;  | |         |,\"\n");
    printf("\t\t\t         /_)______________(_/  //'   | +---------+\n");
    printf("\t\t\t    ___________________________/___  `,\n");
    printf("\t\t\t   /  oooooooooooooooo  .o.  oooo /,   \,\"----------- \n");
    printf("\t\t\t  / ==ooooooooooooooo==.o.  ooo= //   ,`\--{)B     ,\"\n");
    printf("\t\t\t /_==__==========__==_ooo__ooo=_/'   /___________,\"\n");
    printf("\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t***借阅人功能菜单***\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 借阅\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 还书\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t3 - 修改个人信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t4 - 申请续借\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t5 - 罚金情况\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t6 - 已还信息\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t7 - 注销\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(1-6):");
}

//菜单3：管理员菜单
void adminMenu() {
    printf("\n");
    printf("\t\t\t              ,----------------,              ,---------,\n");
    printf("\t\t\t         ,-----------------------,          ,\"        ,\"|\n");
    printf("\t\t\t       ,\"                      ,\"|        ,\"        ,\"  |\n");
    printf("\t\t\t      +-----------------------+  |      ,\"        ,\"    |\n");
    printf("\t\t\t      |  .-----------------.  |  |     +---------+      |\n");
    printf("\t\t\t      |  |                 |  |  |     | -==----'|      |\n");
    printf("\t\t\t      |  |  I LOVE DOS!    |  |  |     |         |      |\n");
    printf("\t\t\t      |  |  Bad command or |  |  |/----|`---=    |      |\n");
    printf("\t\t\t      |  |  C:\\>_          |  |  |   ,/|==== ooo |      ;\n");
    printf("\t\t\t      |  |                 |  |  |  // |(((( [33]|    ,\"\n");
    printf("\t\t\t      |  `-----------------'  |,\" .;\'| |((((     |  ,\"\n");
    printf("\t\t\t      +-----------------------+  ;;  | |         |,\"\n");
    printf("\t\t\t         /_)______________(_/  //'   | +---------+\n");
    printf("\t\t\t    ___________________________/___  `,\n");
    printf("\t\t\t   /  oooooooooooooooo  .o.  oooo /,   \,\"----------- \n");
    printf("\t\t\t  / ==ooooooooooooooo==.o.  ooo= //   ,`\--{)B     ,\"\n");
    printf("\t\t\t /_==__==========__==_ooo__ooo=_/'   /___________,\"\n");
    printf("\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t管理员菜单\t\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 查看个人信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 修改个人信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t3 - 维护图书信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t4 - 查看借书信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t5 - 罚金添加结清\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t6 - 统计借书信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t7 - 备份与还原\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t0 - 注销 \t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(0-6):");
}

//菜单4：图书功能菜单
void bookinforMenu1() {
    printf("\n\n\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t图书信息功能菜单\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 查询图书信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 删除/修改图书信息\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t3 - 新增图书\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t0 - 退出 \t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(0-3):");

}

//菜单5:分类查询菜单
void bookinforMenu2() {

    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t分类查询菜单\t\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 作者名\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 类别\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t3 - 书名\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t4 - isbn号\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t5 - 显示所有图书\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t0 - 退出 \t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(0-5):");

}

//菜单6：统计菜单
void countMenu() {
    printf("\n\n\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t统计菜单\t\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 学院编号\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 性别\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t3 - 年级\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t4 - 年度最受欢迎的图书\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t0 - 退出\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(0-5):");
}

//菜单7：查询借书信息的方式
void bookinforMenu3() {
    printf("\n\n\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t查询借书信息\t\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 借阅人姓名\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 书名\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t0 - 退出\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(0-2):");
}

// 查看罚金
void checkfinesMenu() {
    printf("\n\n\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t罚金操作\t\t\t\t\t║\n");
    printf("\t\t\t╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("\t\t\t║\t\t\t\t1 - 清除罚金\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t2 - 添加罚金\t\t\t\t\t║\n");
    printf("\t\t\t║\t\t\t\t0 - 退出\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\t\t\t 请选择(0-2):");
}

// 管理员登陆
void AdminLoginMenu() {
    printf("\t\t\t               ii.                                         ;9ABH,\n");
    printf("\t\t\t              SA391,                                    .r9GG35&G\n");
    printf("\t\t\t              &#ii13Gh;                               i3X31i;:,rB1\n");
    printf("\t\t\t              iMs,:,i5895,                         .5G91:,:;:s1:8A\n");
    printf("\t\t\t               33::::,,;5G5,                     ,58Si,,:::,sHX;iH1\n");
    printf("\t\t\t                Sr.,:;rs13BBX35hh11511h5Shhh5S3GAXS:.,,::,,1AG3i,GG\n");
    printf("\t\t\t                .G51S511sr;;iiiishS8G89Shsrrsh59S;.,,,,,..5A85Si,h8\n");
    printf("\t\t\t               :SB9s:,............................,,,.,,,SASh53h,1G.\n");
    printf("\t\t\t            .r18S;..,,,,,,,,,,,,,,,,,,,,,,,,,,,,,....,,.1H315199,rX,\n");
    printf("\t\t\t          ;S89s,..,,,,,,,,,,,,,,,,,,,,,,,....,,.......,,,;r1ShS8,;Xi\n");
    printf("\t\t\t        i55s:.........,,,,,,,,,,,,,,,,.,,,......,.....,,....r9&5.:X1\n");
    printf("\t\t\t       59;.....,.     .,,,,,,,,,,,...        .............,..:1;.:&s\n");
    printf("\t\t\t      s8,..;53S5S3s.   .,,,,,,,.,..      i15S5h1:.........,,,..,,:99\n");
    printf("\t\t\t      93.:39s:rSGB@A;  ..,,,,.....    .SG3hhh9G&BGi..,,,,,,,,,,,,.,83\n");
    printf("\t\t\t      G5.G8  9#@@@@@X. .,,,,,,.....  iA9,.S&B###@@Mr...,,,,,,,,..,.;Xh\n");
    printf("\t\t\t      Gs.X8 S@@@@@@@B:..,,,,,,,,,,. rA1 ,A@@@@@@@@@H:........,,,,,,.iX:\n");
    printf("\t\t\t     ;9. ,8A#@@@@@@#5,.,,,,,,,,,... 9A. 8@@@@@@@@@@M;    ....,,,,,,,,S8v\n");
    printf("\t\t\t     X3    iS8XAHH8s.,,,,,,,,,,...,..58hH@@@@@@@@@Hs       ...,,,,,,,:Gs\n");
    printf("\t\t\t    r8,        ,,,...,,,,,,,,,,.....  ,h8XABMMHX3r.          .,,,,,,,.rX:\n");
    printf("\t\t\t   :9, .    .:,..,:;;;::,.,,,,,..          .,,.               ..,,,,,,.59\n");
    printf("\t\t\t  .Si      ,:.i8HBMMMMMB&5,....                    .            .,,,,,.sMr\n");
    printf("\t\t\t  SS       :: h@@@@@@@@@@#; .                     ...  .         ..,,,,iM5\n");
    printf("\t\t\t  91  .    ;:.,1&@@@@@@MXs.                            .          .,,:,:&S\n");
    printf("\t\t\t  hS ....  .:;,,,i3MMS1;..,..... .  .     ...                     ..,:,.99\n");
    printf("\t\t\t  ,8; ..... .,:,..,8Ms:;,,,...                                     .,::.83\n");
    printf("\t\t\t   s&: ....  .sS553B@@HX3s;,.    .,;13h.                            .:::&1\n");
    printf("\t\t\t    SXr  .  ...;s3G99XA&X88Shss11155hi.                             ,;:h&,\n");
    printf("\t\t\t    iH8:  . ..   ,;iiii;,::,,,,,.                                 .;irHA\n");
    printf("\t\t\t      ,8X5;   .     .......                                       ,;iihS8Gi\n");
    printf("\t\t\t         1831,                                                 .,;irrrrrs&@\n");
    printf("\t\t\t           ;5A8r.                                            .:;iiiiirrss1H\n");
    printf("\t\t\t             :X@H3s.......                                .,:;iii;iiiiirsrh\n");
    printf("\t\t\t              r#h:;,...,,.. .,,:;;;;;:::,...              .:;;;;;;iiiirrss1\n");
    printf("\t\t\t             ,M8 ..,....,.....,,::::::,,...         .     .,;;;iiiiiirss11h\n");
    printf("\t\t\t             8B;.,,,,,,,.,.....          .           ..   .:;;;;iirrsss111h\n");
    printf("\t\t\t           i@5,:::,,,,,,,,.... .                   . .:::;;;;;irrrss111111\n");
    printf("\t\t\t           9Bi,:,,,,......                        ..r91;;;;;iirrsss1ss1111\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t   管理员登录\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

}

void ReaderLoginMenu() {
    printf("\t\t\t                                         ,s555SB@@&\n");
    printf("\t\t\t                                      :9H####@@@@@Xi\n");
    printf("\t\t\t                                     1@@@@@@@@@@@@@@8\n");
    printf("\t\t\t                                   ,8@@@@@@@@@B@@@@@@8\n");
    printf("\t\t\t                                  :B@@@@X3hi8Bs;B@@@@@Ah,\n");
    printf("\t\t\t             ,8i                  r@@@B:     1S ,M@@@@@@#8;\n");
    printf("\t\t\t            1AB35.i:               X@@8 .   SGhr ,A@@@@@@@@S\n");
    printf("\t\t\t            1@h31MX8                18Hhh3i .i3r ,A@@@@@@@@@5\n");
    printf("\t\t\t            ;@&i,58r5                 rGSS:     :B@@@@@@@@@@A\n");
    printf("\t\t\t             1#i  . 9i                 hX.  .: .5@@@@@@@@@@@1\n");
    printf("\t\t\t              sG1,  ,G53s.              9#Xi;hS5 3B@@@@@@@B1\n");
    printf("\t\t\t               .h8h.,A@@@MXSs,           #@H1:    3ssSSX@1\n");
    printf("\t\t\t               s ,@@@@@@@@@@@@Xhi,       r#@@X1s9M8    .GA981\n");
    printf("\t\t\t               ,. rS8H#@@@@@@@@@@#HG51;.  .h31i;9@r    .8@@@@BS;i;\n");
    printf("\t\t\t                .19AXXXAB@@@@@@@@@@@@@@#MHXG893hrX#XGGXM@@@@@@@@@@MS\n");
    printf("\t\t\t                s@@MM@@@hsX#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@&,\n");
    printf("\t\t\t              :GB@#3G@@Brs ,1GM@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@B,\n");
    printf("\t\t\t            .hM@@@#@@#MX 51  r;iSGAM@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@8\n");
    printf("\t\t\t          :3B@@@@@@@@@@@&9@h :Gs   .;sSXH@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@:\n");
    printf("\t\t\t      s&HA#@@@@@@@@@@@@@@M89A;.8S.       ,r3@@@@@@@@@@@@@@@@@@@@@@@@@@@r\n");
    printf("\t\t\t   ,13B@@@@@@@@@@@@@@@@@@@5 5B3 ;.         ;@@@@@@@@@@@@@@@@@@@@@@@@@@@i\n");
    printf("\t\t\t  5#@@#&@@@@@@@@@@@@@@@@@@9  .39:          ;@@@@@@@@@@@@@@@@@@@@@@@@@@@;\n");
    printf("\t\t\t  9@@@X:MM@@@@@@@@@@@@@@@#;    ;31.         H@@@@@@@@@@@@@@@@@@@@@@@@@@:\n");
    printf("\t\t\t   SH#@B9.rM@@@@@@@@@@@@@B       :.         3@@@@@@@@@@@@@@@@@@@@@@@@@@5\n");
    printf("\t\t\t     ,:.   9@@@@@@@@@@@#HB5                 .M@@@@@@@@@@@@@@@@@@@@@@@@@B\n");
    printf("\t\t\t           ,ssirhSM@&1;i19911i,.             s@@@@@@@@@@@@@@@@@@@@@@@@@@S\n");
    printf("\t\t\t              ,,,rHAri1h1rh&@#353Sh:          8@@@@@@@@@@@@@@@@@@@@@@@@@#:\n");
    printf("\t\t\t            .A3hH@#5S553&@@#h   i:i9S          #@@@@@@@@@@@@@@@@@@@@@@@@@A.\n");
    printf("\t\t\t╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\t\t\t║\t\t\t\t     读者登录\t\t\t\t\t║\n");
    printf("\t\t\t╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

}