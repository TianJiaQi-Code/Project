#ifndef __G_SESSION_H__
#define __G_SESSION_H__

#include "Util.hpp"

typedef enum
{
    UNLOGIN,
    LOGIN
} ss_statu;

class session
{
public:
    session(uint64_t ssid)
        : _ssid(ssid)
    {
        DEBUG("session %p 被创建!", this);
    }
    ~session()
    {
        DEBUG("session %p 被释放!", this);
    }
    void set_statu(ss_statu statu)
    {
        _statu = statu;
    }
    void set_user(uint64_t uid)
    {
        _uid = uid;
    }
    uint64_t get_user()
    {
        return _uid;
    }
    bool is_login()
    {
        return (_statu == LOGIN);
    }
    void set_timer(const websocket_server::timer_ptr &tp)
    {
        _tp = tp;
    }
    websocket_server::timer_ptr &get_timer()
    {
        return _tp;
    }

private:
    uint64_t _ssid;                  // 标识符
    uint64_t _uid;                   // session对应的用户ID
    ss_statu _statu;                 // 用户状态: 未登录/已登录
    websocket_server::timer_ptr _tp; // session关联的定时器
};

using session_ptr = std::shared_ptr<session>;

class session_manager
{
public:
    session_manager(websocket_server *srv)
        : _next_ssid(1),
          _server(srv)
    {
        DEBUG("session管理器初始化完毕!");
    }
    ~session_manager()
    {
        DEBUG("session管理器即将销毁!");
    }

    session_ptr create_session(uint64_t uid, ss_statu statu)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        session_ptr ssp(new session(_next_ssid));
        ssp->set_statu(statu);
        _session.insert(std::make_pair(_next_ssid, ssp));
        _next_ssid++;
        return ssp;
    }

    session_ptr get_session_by_ssid(uint64_t ssid)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _session.find(ssid);
        if (it == _session.end())
        {
            return session_ptr();
        }
        return it->second;
    }

    void set_session_expire_time(uint64_t ssid, int ms)
    {
    }

    void remove_session(u_int64_t ssid)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _session.erase(ssid);
    }

private:
    uint64_t _next_ssid;
    std::mutex _mutex;
    std::unordered_map<uint64_t, session_ptr> _session;
    websocket_server *_server;
};

#endif