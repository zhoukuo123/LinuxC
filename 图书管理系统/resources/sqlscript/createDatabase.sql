drop database if exists library;
create database if not exists library character set 'utf8';
use library;
create table admin
(
    admin_id       varchar(20) primary key,
    admin_password varchar(20) not null
);
insert into admin
values ('admin', 'admin');
create table reader
(
    reader_id               varchar(20) primary key,
    reader_password         varchar(20) not null,
    reader_name             varchar(20) not null,
    reader_sex              varchar(20) not null,
    reader_grade            varchar(20) not null, # default 1
    reader_telephone        varchar(20) not null,
    reader_department       varchar(20) not null,
    reader_have_borrowed    int         not null, # 已借阅数 0
    reader_left_borrowTimes int         not null, # 可借阅册数(剩下的借阅次数) 20
    reader_left_renewTimes  int         not null, # 可续借次数 2
    reader_penalty          int         not null  # 是否处于罚款中 0
);
insert into reader
values ('abs', '123456', '李明', '1', '1', '18022334455', '1', 0, 20, 2, 0);

create table returned
(
    book_id      varchar(20) not null, # 书号
    reader_id    varchar(20) not null, # id号
    borrow_times datetime    not null, # 借书时间
    return_plan  datetime    not null, # 应还时间
    return_real  datetime    not null  # 实际还书时间
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

