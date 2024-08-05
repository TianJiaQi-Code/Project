#ifndef __G_SERVER_H__
#define __G_SERVER_H__

#include "Room.hpp"
#include "Util.hpp"
#include "DB.hpp"
#include "Online.hpp"
#include "Session.hpp"
#include "Matcher.hpp"

#define WEBROOT "./webroot/"

class gobang_server
{
public:
    // 进行成员初始化, 以及服务器回调函数的设置
    gobang_server(
        const std::string &host,
        const std::string &username,
        const std::string &password,
        const std::string &dbname,
        uint16_t port = 3306,
        const std::string &webroot = WEBROOT)
        : _web_root(webroot),
          _ut(host, username, password, dbname, port),
          _rm(&_ut, &_om),
          _sm(&_wssrv),
          _mm(&_rm, &_ut, &_om)
    {
        _wssrv.set_access_channels(websocketpp::log::alevel::none);
        _wssrv.init_asio();
        _wssrv.set_reuse_addr(true);
        _wssrv.set_http_handler(std::bind(&gobang_server::http_callback, this, std::placeholders::_1));
        _wssrv.set_open_handler(std::bind(&gobang_server::wsopen_callback, this, std::placeholders::_1));
        _wssrv.set_close_handler(std::bind(&gobang_server::wsclose_callback, this, std::placeholders::_1));
        _wssrv.set_message_handler(std::bind(&gobang_server::wsmsg_callback, this, std::placeholders::_1, std::placeholders::_2));
    }

    // 启动服务器
    void start(int port)
    {
        _wssrv.listen(port);
        _wssrv.start_accept();
        _wssrv.run();
    }

private:
    // 静态资源请求的处理
    void file_handler(websocket_server::connection_ptr &conn)
    {
        // 1. 获取到请求uri-资源路径, 了解客户端请求的页面文件名称
        websocketpp::http::parser::request req = conn->get_request();
        std::string uri = req.get_uri();
        // 2. 组合出文件的实际路径: 根目录+uri
        std::string realpath = _web_root + uri;
        // 3. 如果请求的是个目录, 增加一个后缀login.html
        if (realpath.back() == '/')
        {
            realpath += "login.html";
        }
        // 4. 读取文件内容
        Json::Value resp_json;
        std::string body;
        bool ret = file_util::read(realpath, body);
        // 4.1 文件不存在, 读取文件内容失败, 返回404
        if (ret == false)
        {
            body += "<html>";
            body += "<head>";
            body += "<meta charset='UTF-8'/>";
            body += "</head>";
            body += "<body>";
            body += "<h1> Not Found </h1>";
            body += "</body>";
            body += "</html>";
            conn->set_status(websocketpp::http::status_code::not_found);
            conn->set_body(body);
            return;
        }
        // 5. 设置响应正文
        conn->set_body(body);
        conn->set_status(websocketpp::http::status_code::ok);
    }

    void http_resp(bool result, websocketpp::http::status_code::value code, const std::string &reason)
    {
        Json::Value resp_json;
        resp_json["result"] = result;
        resp_json["reason"] = reason;
        std::string resp_body;
        json_util::serialize(resp_json, resp_body);
        conn->set_status(code);
        conn->set_body(resp_body);
        conn->append_header("Content-Type", "application/json");
        return;
    }

    // 用户注册功能请求的处理
    void reg(websocket_server::connection_ptr &conn)
    {
        // 1. 获取到请求正文
        websocketpp::http::parser::request req = conn->get_request();
        std::string req_body = conn->get_request_body();
        // 2. 对正文进行json反序列化, 得到用户名和密码
        Json::Value login_info;
        Json::Value resp_json;
        bool ret = json_util::unserialize(req_body, login_info);
        if (ret == false)
        {
            return http_resp(false, websocketpp::http::status_code::bad_request, "请求的正文格式错误");
        }
        // 3. 进行数据库的用户新增操作
        if (login_info["username"].isNull() || login_info["password"].isNull())
        {
            return http_resp(false, websocketpp::http::status_code::bad_request, "请输入用户名/密码");
        }
        ret = _ut.insert(login_info);
        if (ret == false)
        {
            return http_resp(false, websocketpp::http::status_code::bad_request, "用户名已经被占用");
        }
        // 4. 如果成功了, 则返回200
        return http_resp(true, websocketpp::http::status_code::ok, "用户注册成功");
    }

    // 用户登录功能请求的处理
    void login(websocket_server::connection_ptr &conn)
    {
    }
    // 用户信息获取功能请求的处理
    void info(websocket_server::connection_ptr &conn)
    {
    }
    void http_callback(websocketpp::connection_hdl hdl)
    {
        websocket_server::connection_ptr conn = _wssrv.get_con_from_hdl(hdl);
        websocketpp::http::parser::request req = conn->get_request();
        std::string method = req.get_method();
        std::string uri = req.get_uri();
        if (method == "POST" && uri == "/reg")
        {
            return reg(conn);
        }
        else if (method == "POST" && uri == "/login")
        {
            return login(conn);
        }
        else if (method == "GET" && uri == "/info")
        {
            return info(conn);
        }
        else
        {
            return file_handler(conn);
        }
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
    websocket_server _wssrv;
    user_table _ut;
    online_manager _om;
    room_manager _rm;
    matcher _mm;
    session_manager _sm;
};

#endif