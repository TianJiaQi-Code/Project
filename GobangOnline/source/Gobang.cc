#include <iostream>
#include <vector>
#include "../logs/Log.h"
#include "Util.hpp"
#include "DB.hpp"
#include "Online.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "Matcher.hpp"

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

void test_db_h()
{
    user_table ut(HOST, USER, PASS, DBNAME, PORT);
    Json::Value user;
    // user["username"] = "xiaotian";
    // user["password"] = "123456";
    // ut.insert(user);

    // bool ret = ut.login(user);
    // if (ret == false)
    // {
    //     DEBUG("login failed!");
    // }

    // bool ret = ut.select_by_name("xiaotian", user);
    // bool ret = ut.select_by_id(2, user);
    // std::string body;
    // json_util::serialize(user, body);
    // DEBUG(body.c_str());

    // ut.win(1);
    // ut.lose(1);

    user["username"] = "xiaoming";
    ut.insert(user);
}

void test_online_h()
{
    online_manager om;
    websocket_server::connection_ptr conn;
    uint64_t uid = 2;
    // om.enter_game_hall(uid, conn);
    // if (om.is_in_game_hall(uid))
    // {
    //     DEBUG("in game hall");
    // }
    // else
    // {
    //     DEBUG("not in game hall");
    // }
    // om.exit_game_hall(uid);
    // if (om.is_in_game_hall(uid))
    // {
    //     DEBUG("in game hall");
    // }
    // else
    // {
    //     DEBUG("not in game hall");
    // }

    om.enter_game_room(uid, conn);
    if (om.is_in_game_room(uid))
    {
        DEBUG("in game room");
    }
    else
    {
        DEBUG("not in game room");
    }
    om.exit_game_room(uid);
    if (om.is_in_game_room(uid))
    {
        DEBUG("in game room");
    }
    else
    {
        DEBUG("not in game room");
    }
}

void test_room_h()
{
    user_table ut(HOST, USER, PASS, DBNAME, PORT);
    online_manager om;
    // room r(10, &ut, &om);
    room_manager rm(&ut, &om);
    room_ptr rp = rm.create_room(10, 20);
}

void test_matcher_h()
{
    user_table ut(HOST, USER, PASS, DBNAME, PORT);
    online_manager om;
    room_manager rm(&ut, &om);
    matcher mc(&rm, &ut, &om);
}

int main()
{
    test_matcher_h();
    // test_room_h();
    // test_online_h();
    // test_db_h();
    // test_file_util();
    // test_string_util();
    // test_json_util();
    // test_mysql_util();
    return 0;
}