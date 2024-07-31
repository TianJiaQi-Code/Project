#ifndef __G_UTIL_H__
#define __G_UTIL_H__

#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>
#include "../logs/Log.h"

class mysql_util
{
public:
    static MYSQL *mysql_create(
        const std::string &host,
        const std::string &username,
        const std::string &password,
        const std::string &dbname,
        uint16_t port = 3306)
    {
        //  1. 初始化mysql句柄
        MYSQL *mysql = mysql_init(NULL);
        if (mysql == NULL)
        {
            ERROR("mysql init failed!");
            return NULL;
        }
        //  2. 连接服务器
        if (mysql_real_connect(mysql, host.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, NULL, 0) == NULL)
        {
            ERROR("connect mysql server failed: %s", mysql_error(mysql));
            mysql_close(mysql);
            return NULL;
        }
        //  3. 设置客户端字符集
        if (mysql_set_character_set(mysql, "utf8") != 0)
        {
            ERROR("set client character failed: %s", mysql_error(mysql));
            mysql_close(mysql);
            return NULL;
        }
        return mysql;
    }
    static bool mysql_exec(MYSQL *mysql, const std::string &sql)
    {
        int ret = mysql_query(mysql, sql.c_str());
        if (ret != 0)
        {
            ERROR("%s", sql.c_str());
            ERROR("mysql query failed: %s", mysql_error(mysql));
            return false;
        }
        return true;
    }
    static void mysql_destroy(MYSQL *mysql)
    {
        if (mysql != NULL)
        {
            mysql_close(mysql);
        }
        return;
    }
};

class json_util
{
public:
    static bool serialize(const Json::Value &root, std::string &str)
    {
        // 1. 实例化一个StreamWriterBuilder工厂类对象
        Json::StreamWriterBuilder swb;
        // 2. 通过StreamWriterBuilder工厂类对象生产一个StreamWriter对象
        std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
        // 3. 使用StreamWriter对象, 对Json::Value中存储的数据进行序列化
        std::stringstream ss;
        int ret = sw->write(root, &ss);
        if (ret != 0)
        {
            ERROR("json serialize failed!");
            return false;
        }
        str = ss.str();
        return true;
    }
    static bool unserialize(const std::string &str, Json::Value &root)
    {
        // 1. 实例化一个CharReaderBuilder工厂类对象
        Json::CharReaderBuilder crb;
        // 2. 使用CharReaderBuilder工厂生产一个CharReader对象
        std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
        // 3. 使用CharReader对象进行json格式字符串str的反序列化
        std::string err;
        bool ret = cr->parse(str.c_str(), str.c_str() + str.size(), &root, &err);
        if (ret == false)
        {
            ERROR("json unserialize failed: %s", err.c_str());
            return false;
        }
        return true;
    }
};

class string_util
{
public:
    static int split(const std::string &src, const std::string &sep, std::vector<std::string> &res)
    {
        size_t pos, idx = 0;
        while (idx < src.size())
        {
            pos = src.find(sep, idx);
            if (pos == std::string::npos)
            {
                // 没有找到, 字符串中没有间隔字符了, 则跳出循环
                res.push_back(src.substr(idx));
                break;
            }
            if (pos == idx)
            {
                idx += sep.size();
                continue;
            }
            res.push_back(src.substr(idx, pos - idx));
            idx = pos + sep.size();
        }
        return res.size();
    }
};

class file_util
{
public:
    static bool read(const std::string &filename, std::string &body)
    {
        // 1. 打开文件
        std::ifstream ifs(filename, std::ios::binary);
        if (ifs.is_open() == false)
        {
            ERROR("%s file open failed!", filename.c_str());
            return false;
        }
        // 2. 获取文件大小
        size_t fsize = 0;
        ifs.seekg(0, std::ios::end);
        fsize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        body.resize(fsize);
        // 3. 将文件所有数据读取出来
        ifs.read(&body[0], fsize);
        if (ifs.good() == false)
        {
            ERROR("read %s file content failed!", filename.c_str());
            ifs.close();
            return false;
        }
        // 4. 关闭文件
        ifs.close();
        return true;
    }
};

#endif