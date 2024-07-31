#ifndef __G_DB_H__
#define __G_DB_H__

#define INSERT_USER "insert user values(null, '%s', password('%s'), 1000, 0, 0);"
// 以用户名和密码共同作为查询过滤条件, 查询到数据则表示用户名密码一致, 没有信息则用户名密码错误
#define LOGIN_USER "select id, score, total_count, win_count from user where username='%s' and password=password('%s');"
#define USER_BY_NAME "select id, score, total_count, win_count from user where username='%s';"
#define USER_BY_ID "select username, score, total_count, win_count from user where id=%ld;"
#define USER_WIN "update user set score=score+30, total_count=total_count+1, win_count=win_count+1 where id=%ld;"
#define USER_LOSE "update user set score=score-30, total_count=total_count+1 where id=%ld;"

#include <mutex>
#include <cassert>
#include "Util.hpp"

class user_table
{
public:
    user_table(
        const std::string &host,
        const std::string &username,
        const std::string &password,
        const std::string &dbname,
        uint16_t port = 3306)
    {
        _mysql = mysql_util::mysql_create(host, username, password, dbname, port);
        assert(_mysql != NULL);
    }

    ~user_table()
    {
        mysql_util::mysql_destroy(_mysql);
        _mysql = NULL;
    }

    // 注册时新增用户
    bool insert(Json::Value &user)
    {
        if (user["password"].isNull() || user["username"].isNull())
        {
            DEBUG("input password or username!");
            return false;
        }
        char sql[4096] = {0};
        sprintf(sql, INSERT_USER, user["username"].asCString(), user["password"].asCString());
        bool ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            DEBUG("insert user info failed!");
            return false;
        }
        return true;
    }

    // 登录验证, 并返回详细的用户信息
    bool login(Json::Value &user)
    {
        if (user["password"].isNull() || user["username"].isNull())
        {
            DEBUG("input password or username!");
            return false;
        }
        char sql[4096] = {0};
        sprintf(sql, LOGIN_USER, user["username"].asCString(), user["password"].asCString());
        MYSQL_RES *res = NULL;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool ret = mysql_util::mysql_exec(_mysql, sql);
            if (ret == false)
            {
                DEBUG("user login failed!");
                return false;
            }
            // 要么有数据, 要么没有数据, 就算有数据也只能有一条数据
            res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                DEBUG("have no login user info!");
                return false;
            }
        }
        int row_num = mysql_num_rows(res);
        if (row_num != 1)
        {
            DEBUG("the user information queried is not unique!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        user["id"] = (Json::UInt64)std::stol(row[0]);
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);
        mysql_free_result(res);
        return true;
    }

    // 通过用户名获取用户信息
    bool select_by_name(const std::string &name, Json::Value &user)
    {
        char sql[4096] = {0};
        sprintf(sql, USER_BY_NAME, name.c_str());
        MYSQL_RES *res = NULL;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool ret = mysql_util::mysql_exec(_mysql, sql);
            if (ret == false)
            {
                DEBUG("get user by name failed!");
                return false;
            }
            // 要么有数据, 要么没有数据, 就算有数据也只能有一条数据
            res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                DEBUG("have no user info!");
                return false;
            }
        }
        int row_num = mysql_num_rows(res);
        if (row_num != 1)
        {
            DEBUG("the user information queried is not unique!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        user["id"] = (Json::UInt64)std::stol(row[0]);
        user["username"] = name;
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);
        mysql_free_result(res);
        return true;
    }

    // 通过id获取用户信息
    bool select_by_id(uint64_t id, Json::Value &user)
    {
        char sql[4096] = {0};
        sprintf(sql, USER_BY_ID, id);
        MYSQL_RES *res = NULL;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool ret = mysql_util::mysql_exec(_mysql, sql);
            if (ret == false)
            {
                DEBUG("get user by id failed!");
                return false;
            }
            // 要么有数据, 要么没有数据, 就算有数据也只能有一条数据
            res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                DEBUG("have no user info!");
                return false;
            }
        }
        int row_num = mysql_num_rows(res);
        if (row_num != 1)
        {
            DEBUG("the user information queried is not unique!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        user["id"] = (Json::UInt64)id;
        user["username"] = row[0];
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);
        mysql_free_result(res);
        return true;
    }

    // 胜利时天梯分数增加30, 战斗场次增加1, 胜利场次增加1
    bool win(uint64_t id)
    {
        char sql[4096] = {0};
        sprintf(sql, USER_WIN, id);
        bool ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            DEBUG("update win user info failed!");
            return false;
        }
        return true;
    }

    // 失败时天梯分数减少30, 战斗场次增加1, 其他不变
    bool lose(uint64_t id)
    {
        char sql[4096] = {0};
        sprintf(sql, USER_LOSE, id);
        bool ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            DEBUG("update lose user info failed!");
            return false;
        }
        return true;
    }

private:
    MYSQL *_mysql;     // mysql操作句柄
    std::mutex _mutex; // 互斥锁保护数据库的访问操作
};

#endif