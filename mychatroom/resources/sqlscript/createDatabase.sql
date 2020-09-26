CREATE DATABASE IF NOT EXISTS chatroom CHARACTER SET 'utf8';
USE chatroom;


DROP TABLE account;
CREATE TABLE account
(
    id                INT(20) AUTO_INCREMENT PRIMARY KEY,
    passwd            VARCHAR(40) NOT NULL,
    user_name         VARCHAR(20) NOT NULL,
    security_question VARCHAR(20) NOT NULL,
    security_answer   VARCHAR(20) NOT NULL,
    online            INT(10)     NOT NULL # 是否在线, 1在线, 0不在线
) ENGINE = InnoDB
  AUTO_INCREMENT = 10000;


drop table friend;
CREATE TABLE friend
(
    user_id      varchar(20) not null,
    friend_id    varchar(20) not null,
    relationship int         not null # 1 普通好友 2 黑名单
);


drop table chat_message;
create table chat_message
(
    user_id     varchar(20) not null,
    user_name   varchar(20) not null,
    friend_id   varchar(20) not null,
    friend_name varchar(20) not null,
    message     varchar(30) not null
);

create table group_outline
(
    group_id      INT(20) AUTO_INCREMENT PRIMARY KEY,
    group_name    varchar(20) not null,
    group_mem_num varchar(20) not null
) ENGINE = InnoDB
  AUTO_INCREMENT = 10000;

drop table group_member;
create table group_member
(
    id         varchar(20) not null,
    name       varchar(20) not null,
    gourp_id   INT(20)     not null,
    group_name varchar(20) not null,
    status     int         not null # 1 群主 2为管理员 3 普通群员
);

