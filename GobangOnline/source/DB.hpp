#ifndef __G_DB_H__
#define __G_DB_H__

#include "Util.hpp"

#define INSERT_USER "insert user values(null, '%s', password('%s'), 1000, 0, 0);"

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
        Json::Value val;
        bool ret = select_by_name(user["username"].asCString(), val);
        if (ret == true)
        {
            DEBUG("user: %s is already exists", user["username"].asCString());
            return false;
        }
        char sql[4096] = {0};
        sprintf(sql, INSERT_USER, user["username"].asCString(), user["password"].asCString());
        ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            ERROR("insert user info failed!");
            return false;
        }
        return true;
    }
    // 登录验证, 并返回详细的用户信息
    bool login(Json::Value &user);
    // 通过用户名获取用户信息
    bool select_by_name(const std::string &name, Json::Value &user);
    // 通过id获取用户信息
    bool select_by_id(int id, Json::Value &user);
    // 胜利时天梯分数增加, 战斗场次增加, 胜利场次增加
    bool win(int id);
    // 失败时天梯分数减少, 战斗场次增加, 其他不变
    bool lose(int id);

private:
    MYSQL *_mysql;     // mysql操作句柄
    std::mutex _mutex; // 互斥锁保护数据库的访问操作
};

#endif