drop database if exists mychatroom;
create database if not exists mychatroom character set 'utf8';
use mychatroom;

create table user_data # 用户数据表
(
    account     varchar(20) not null primary key,
    nickname    varchar(20) not null,
    password    varchar(20) not null,
    user_status int         not null, # 用户状态(是否在线)
    user_socket int         not null
);

create table `groups` # 群信息表
(
    group_account       varchar(20) not null primary key, # 群号
    group_name          varchar(20) not null,             # 群名
    group_member_number datetime    not null              # 群成员数量
);

create table group_members # 群成员表
(
    group_account         varchar(20) not null primary key, # 群号
    group_name            varchar(20) not null,             # 群名
    group_member_account  varchar(20) not null,             # 群成员账号
    group_member_nickname varchar(20) not null,             # 群成员昵称
    group_state           varchar(20) not null              # 群成员群地位(群主, 管理员, 普通群员)
);

create table friends # 好友表
(
    user        varchar(20) not null primary key, # 当前用户账号
    friend_user varchar(20) not null,             # 该用户好友账号
    realtion    int         not null              # 两人的关系(特别关心,黑名单,普通)
);

create table chat_messages # 聊天信息表
(
    send_user varchar(20) not null, # 发送者的账号
    recv_user varchar(20) not null, # 接收者的账号
    messages  varchar(30) not null  # 消息内容
);
