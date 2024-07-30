-- 数据管理模块
-- 1. 数据库表的设计
create database if not exists gobang;
use gobang;
create table if not exists user(
    id int primary key auto_increment,
    username varchar(32) not null,
    password varchar(32) not null,
    score int,
    total_count int,
    win_count int
);