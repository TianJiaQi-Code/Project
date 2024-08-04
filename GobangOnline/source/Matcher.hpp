#ifndef __G_MATCHER_H__
#define __G_MATCHER_H__

#include <list>
#include <mutex>
#include <condition_variable>
#include "Room.hpp"
#include "Util.hpp"
#include "DB.hpp"
#include "Online.hpp"

template <class T>
class match_queue
{
public:
    // 获取元素个数
    int size()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _list.size();
    }
    // 判断是否为空
    bool empty()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _list.empty();
    }
    // 阻塞线程
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock);
    }
    // 入队数据, 并唤醒线程
    void push(const T &data)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _list.push_back(data);
        _cond.notify_all();
    }
    // 出队数据
    bool pop(T &data)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_list.empty() == true)
        {
            return false;
        }
        data = _list.front();
        _list.pop_front();
        return true;
    }
    // 移除指定的数据
    void remove(T &data)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _list.remove(data);
    }

private:
    // 用链表而不直接使用queue是因为我们有中间删除数据的需要
    std::list<T> _list;
    // 实现线程安全
    std::mutex _mutex;
    // 这个条件变量主要为了阻塞消费者, 后边使用的时候, 队列中元素个数<2则阻塞
    std::condition_variable _cond;
};

class matcher
{
public:
    matcher(room_manager *rm, user_table *ut, online_manager *om)
        : _rm(rm),
          _ut(ut),
          _om(om),
          _th_bronze(std::thread(&matcher::_th_bronze_entry, this)),
          _th_silver(std::thread(&matcher::_th_silver_entry, this)),
          _th_gold(std::thread(&matcher::_th_gold_entry, this))
    {
        DEBUG("游戏匹配模块初始化完毕...");
    }

    // 根据玩家的天梯分数, 来判定玩家档次, 添加到不同的匹配队列
    bool add(uint64_t uid)
    {
        // 1. 根据用户ID, 获取玩家信息
        Json::Value user;
        bool ret = _ut->select_by_id(uid, user);
        if (ret == false)
        {
            DEBUG("获取玩家: %d 信息失败!", uid);
            return false;
        }
        // 2. 添加到指定的队列中
        int score = user["score"].asInt();
        if (score < 2000)
        {
            _q_bronze.push(uid);
        }
        else if (score >= 2000 && score < 3000)
        {
            _q_silver.push(uid);
        }
        else
        {
            _q_gold.push(uid);
        }
        return true;
    }

    bool del(uint64_t uid)
    {
        // 1.
        Json::Value user;
        bool ret = _ut->select_by_id(uid, user);
        if (ret == false)
        {
            DEBUG("获取玩家: %d 信息失败!", uid);
            return false;
        }
        // 2.
        int score = user["score"].asInt();
        if (score < 2000)
        {
            _q_bronze.remove(uid);
        }
        else if (score >= 2000 && score < 3000)
        {
            _q_silver.remove(uid);
        }
        else
        {
            _q_gold.remove(uid);
        }
        return true;
    }

private:
    void handle_match(match_queue<uint64_t> &mq)
    {
        while (true)
        {
            // 1. 判断队列人数是否大于2, <2则阻塞等待
            while (mq.size() < 2)
            {
                mq.wait();
            }
            // 2. 走下来代表人数够了, 出队两个玩家
            uint64_t uid1, uid2;
            bool ret = mq.pop(uid1);
            if (ret == false)
            {
                continue;
            }
            ret = mq.pop(uid2);
            if (ret == false)
            {
                this->add(uid1);
                continue;
            }
            // 3. 校验两个玩家是否在线, 如果有人掉线, 则要把另一个人重新添加入队列
            websocket_server::connection_ptr conn1 = _om->get_conn_from_hall(uid1);
            if (conn1.get() == nullptr)
            {
                this->add(uid2);
                continue;
            }
            websocket_server::connection_ptr conn2 = _om->get_conn_from_hall(uid2);
            if (conn2.get() == nullptr)
            {
                this->add(uid1);
                continue;
            }
            // 4. 为两个玩家创建房间, 并将玩家加入房间中
            room_ptr rp = _rm->create_room(uid1, uid2);
            if (rp.get() == nullptr)
            {
                this->add(uid1);
                this->add(uid2);
                continue;
            }
            // 5. 对两个玩家进行响应
            Json::Value resp;
            resp["optype"] = "match_success";
            resp["result"] = true;
            std::string body;
            json_util::serialize(resp, body);
            conn1->send(body);
            conn2->send(body);
        }
    }

    void _th_bronze_entry()
    {
        return handle_match(_q_bronze);
    }
    void _th_silver_entry()
    {
        return handle_match(_q_silver);
    }
    void _th_gold_entry()
    {
        return handle_match(_q_gold);
    }

private:
    match_queue<uint64_t> _q_bronze; // 青铜匹配队列
    match_queue<uint64_t> _q_silver; // 白银匹配队列
    match_queue<uint64_t> _q_gold;   // 黄金匹配队列
    std::thread _th_bronze;
    std::thread _th_silver;
    std::thread _th_gold;
    room_manager *_rm;
    user_table *_ut;
    online_manager *_om;
};

#endif