#include <iostream>
#include <vector>
#include "../logs/Log.h"
#include "Util.hpp"

#define HOST "127.0.0.1"
#define PORT 3306
#define USER "root"
#define PASS "Abc13546137306"
#define DBNAME "gobang"

void test_mysql_util()
{
    MYSQL *mysql = mysql_util::mysql_create(HOST, USER, PASS, DBNAME, PORT);
    const char *sql = "insert stu values(null, '小田', 20, 53, 68, 87);";
    bool ret = mysql_util::mysql_exec(mysql, sql);
    if (ret == false)
    {
        return;
    }
    mysql_util::mysql_destroy(mysql);
}

void test_json_util()
{
    Json::Value root;
    std::string body;
    root["姓名"] = "小田";
    root["年龄"] = 20;
    root["成绩"].append(98);
    root["成绩"].append(88.5);
    root["成绩"].append(78.5);
    json_util::serialize(root, body);
    DEBUG(body.c_str());

    Json::Value val;
    json_util::unserialize(body, val);
    std::cout << "姓名: " << root["姓名"].asString() << std::endl;
    std::cout << "年龄: " << root["年龄"].asInt() << std::endl;
    int sz = root["成绩"].size();
    for (int i = 0; i < sz; i++)
    {
        std::cout << "成绩: " << root["成绩"][i].asFloat() << std::endl;
    }
}

void test_string_util()
{
    std::string str = "...,123,234,,,,,345";
    std::vector<std::string> arr;
    string_util::split(str, ",", arr);
    for (auto &s : arr)
    {
        DEBUG("%s", s.c_str());
    }
}

void test_file_util()
{
    std::string filename = "./Makefile";
    std::string body;
    file_util::read(filename, body);
    DEBUG(body.c_str());
}

int main()
{
    test_file_util();
    // test_string_util();
    // test_json_util();
    // test_mysql_util();
    return 0;
}