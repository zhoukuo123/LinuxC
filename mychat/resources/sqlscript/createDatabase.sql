drop database if exists mychatroom;
create database if not exists mychatroom character set 'utf8';
use mychatroom;

create table user_data
(
    account     varchar(20) primary key,
    nickname    varchar(20) not null,
    password    varchar(20) not null,
    user_status int         not null,
    user_socket int         not null,
);

insert into user_data
values ('1178053077', '123456');

create table friends
(
    friends_id       varchar(20) primary key,
    friends_nickname varchar(20) not null,
    friends_status   int         not null,
    friends_socket   int         not null,
);

insert into friends
values ('abs', '123456', '李明', '1', '1', '18022334455', '1', 0, 20, 2, 0);

create table group
(
    group_account         varchar(20) not null,
    group_name            varchar(20) not null,
    group_member_account  datetime    not null,
    group_member_nickname datetime    not null,
    return_real           datetime    not null
);

create table borrowed
(
    book_id      varchar(20) not null, # 书号
    reader_id    varchar(20) not null, # id号
    borrow_times datetime    not null, # 借书时间
    return_plan  datetime    not null, # 应还时间
    valid        int         not null  # 是否有效
);

create table bookshelf
(
    bookshelf int primary key,     # 书架号
    class     varchar(20) not null # 类别
);

insert into bookshelf
values (1, 'Linux'),
       (2, '深度学习'),
       (3, '机器学习'),
       (4, '英语');

create table schoolcard
(
    school_id       varchar(20) primary key, # 校园id号
    school_password varchar(20)  not null,   # 校园id密码
    left_money      double(5, 2) not null,   # 类别
    locked          int          not null    # 是否被锁
);
insert into schoolcard
values ('20194740', '123456', 100.00, 0),
       ('20195740', '123456', 100.00, 0),
       ('20196740', '123456', 100.00, 0);

create table book
(
    book_id   varchar(20) primary key, # 书号
    book_name varchar(50)   not null,  # 书名
    isbn      varchar(20)   not null,  # isbn号
    writer    varchar(20)   not null,  # 作者
    price     double(10, 2) not null,  # 价格
    publish   varchar(20)   not null,  # 出版社
    location  int           not null,  # 存放书架号
    borrowed  int           not null,  # 被借
    info      varchar(50)   not null   # 摘要
);

insert into book
values ('000001', 'Linux操作系统', '978-7-310-05677-4', '郭思延', '59.00', '南开大学出版社', 1, 0, '系统而全面, 讲解Linux的基础知识、系统管理、服务器搭建');

delimiter $$
create trigger T
    BEFORE insert
    on book
    for each row
begin
    set new.book_id = lpad(((select substring(book_id, 1, 6)
                             from book
                             where book_id = (select book_id from book order by book_id desc limit 1)) + 1), 6, '0');
end $$

insert into book
values (0, 'Linux操作系统', '978-7-310-05677-4', '郭思延', '59.00', '南开大学出版社', 1, 0, '系统而全面, 讲解Linux的基础知识、系统管理、服务器搭建'),
       (0, 'Linux Shell核心编程指南', '978-7-121-37571-2', '丁明一', '89.00', '电子工业出版社', 1, 0,
        '本书将围绕Linux系统中最常用的Shell脚本语言, 讲解如何通过Shell编写自动化、智能化脚本'),
       (0, 'Linux Shell核心编程指南', '978-7-121-37571-2', '丁明一', '89.00', '电子工业出版社', 1, 0,
        '本书将围绕Linux系统中最常用的Shell脚本语言, 讲解如何通过Shell编写自动化、智能化脚本'),
       (0, '面向自然语言处理的深度学习', '978-7-111-61719-8', '戈雅尔', '69.00', '机械工业出版社', 2, 0, '面向自然语言处理的深度学习'),
       (0, '面向自然语言处理的深度学习', '978-7-111-61719-8', '戈雅尔', '69.00', '机械工业出版社', 2, 0, '面向自然语言处理的深度学习'),
       (0, 'Python机器学习手册', '978-7-121-36962-9', '阿尔本', '89.00', '南开大学出版社', 3, 0, '采用基于任务的方式来介绍如何在机器学习中使用Python'),
       (0, 'Python机器学习手册', '978-7-121-36962-9', '阿尔本', '89.00', '南开大学出版社', 3, 0, '采用基于任务的方式来介绍如何在机器学习中使用Python'),
       (0, '托福综合写作高分范文', '978-7-5159-1652-1', '谢侃', '39.80', '中国宇航出版社', 4, 0, '本书涵盖50道TPO综合写作题目, 对每道试题进行阅读和听力要点概括'),
       (0, '托福综合写作高分范文', '978-7-5159-1652-1', '谢侃', '39.80', '中国宇航出版社', 4, 0, '本书涵盖50道TPO综合写作题目, 对每道试题进行阅读和听力要点概括');


