#ifndef __G_SERVER_H__
#define __G_SERVER_H__

#include "Room.hpp"
#include "Util.hpp"
#include "DB.hpp"
#include "Online.hpp"
#include "Session.hpp"
#include "Matcher.hpp"

class gobang_server
{
public:
    // 进行成员初始化, 以及服务器回调函数的设置
    gobang_server()
    {
        _wssrv.set_access_channels(websocketpp::log::alevel::none);
        _wssrv.init_asio();
        _wssrv.set_reuse_addr(true);
        _wssrv.set_http_handler(std::bind(http_callback, this, std::placeholders::_1));
        _wssrv.set_open_handler(std::bind(wsopen_callback, this, std::placeholders::_1));
        _wssrv.set_close_handler(std::bind(wsclose_callback, this, std::placeholders::_1));
        _wssrv.set_message_handler(std::bind(wsmsg_callback, this, std::placeholders::_1, std::placeholders::_2));
    }

    // 启动服务器
    void start(int port)
    {
        _wssrv.listen(port);
        _wssrv.start_accept();
        _wssrv.run();
    }

private:
    void http_callback(websocketpp::connection_hdl hdl)
    {
    }
    void wsopen_callback(websocketpp::connection_hdl hdl)
    {
    }
    void wsclose_callback(websocketpp::connection_hdl hdl)
    {
    }
    void wsmsg_callback(websocketpp::connection_hdl hdl, websocket_server::message_ptr msg)
    {
    }

private:
    std::string _web_root; // 静态资源根目录
    user_table _ut;
    matcher _mm;
    online_manager _om;
    room_manager _rm;
    session_manager _sm;
    websocket_server _wssrv;
};

#endif