drop database if exists mychatroom;
create database if not exists mychatroom character set 'utf8';
use mychatroom;

create table user_data
(
    account     varchar(20) primary key,
    nickname    varchar(20) not null,
    password    varchar(20) not null,
    user_status int         not null,
    user_socket int         not null
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
    group_account       varchar(20) not null,
    group_name          varchar(20) not null,
    group_member_number datetime    not null,
);

