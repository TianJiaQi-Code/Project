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

    void http_resp(websocket_server::connection_ptr &conn, bool result, websocketpp::http::status_code::value code, const std::string &reason)
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
        bool ret = json_util::unserialize(req_body, login_info);
        if (ret == false)
        {
            DEBUG("反序列化注册信息失败");
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "请求的正文格式错误");
        }
        // 3. 进行数据库的用户新增操作
        if (login_info["username"].isNull() || login_info["password"].isNull())
        {
            DEBUG("用户名/密码不完整");
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "请输入用户名/密码");
        }
        ret = _ut.insert(login_info);
        if (ret == false)
        {
            DEBUG("向数据库插入数据失败");
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "用户名已经被占用");
        }
        // 4. 如果成功了, 则返回200
        return http_resp(conn, true, websocketpp::http::status_code::ok, "用户注册成功");
    }

    // 用户登录功能请求的处理
    void login(websocket_server::connection_ptr &conn)
    {
        // 1. 获取请求正文, 并进行json反序列化, 得到用户名和密码
        websocketpp::http::parser::request req = conn->get_request();
        std::string req_body = conn->get_request_body();
        Json::Value login_info;
        bool ret = json_util::unserialize(req_body, login_info);
        if (ret == false)
        {
            DEBUG("反序列化登录信息失败");
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "请求的正文格式错误");
        }
        // 2. 校验正文完整性, 进行数据库的用户信息验证
        if (login_info["username"].isNull() || login_info["password"].isNull())
        {
            DEBUG("用户名/密码不完整");
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "请输入用户名/密码");
        }
        ret = _ut.login(login_info);
        // 2.1 如果验证失败, 则返回400
        if (ret == false)
        {
            DEBUG("用户名/密码错误");
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "用户名/密码错误");
        }
        // 3. 如果验证成功, 给客户端创建session
        uint64_t uid = login_info["id"].asUInt64();
        session_ptr ssp = _sm.create_session(uid, LOGIN);
        if (ssp.get() == nullptr)
        {
            DEBUG("创建会话失败");
            return http_resp(conn, false, websocketpp::http::status_code::internal_server_error, "创建会话失败");
        }
        _sm.set_session_expire_time(ssp->ssid(), SESSION_TIMEOUT);
        // 4. 设置响应头部, Set-Cookie, 将session通过cookie返回
        std::string cookie_ssid = "SSID=" + std::to_string(ssp->ssid());
        conn->append_header("Set-Cookie", cookie_ssid);
        return http_resp(conn, true, websocketpp::http::status_code::ok, "登录成功");
    }

    bool get_cookie_val(const std::string &cookie_str, const std::string &key, std::string &val)
    {
        // Cookie: ssid=xxx, path=/
        // 1. 以 "; " 作为间隔, 对字符串进行分割, 得到各个单个的cookie信息
        std::string sep = "; ";
        std::vector<std::string> cookie_arr;
        string_util::split(cookie_str, sep, cookie_arr);
        for (auto str : cookie_arr)
        {
            // 2. 对单个cookie字符串, 以 "=" 为间隔符进行分割, 得到key和val
            std::vector<std::string> tmp_arr;
            string_util::split(str, "=", tmp_arr);
            if (tmp_arr.size() != 2)
            {
                continue;
            }
            if (tmp_arr[0] == key)
            {
                val = tmp_arr[1];
                return true;
            }
        }
        return false;
    }

    // 用户信息获取功能请求的处理
    void info(websocket_server::connection_ptr &conn)
    {
        Json::Value err_resp;
        // 1. 获取请求信息中的cookie, 从cookie中获取ssid
        std::string cookie_str = conn->get_request_header("Cookie");
        if (cookie_str.empty())
        {
            // 如果没有cookie, 返回错误: 没有cookie信息, 让客户端重新登录
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "找不到cookie信息, 请重新登录");
        }
        // 1.5 从cookie中取出ssid
        std::string ssid_str;
        bool ret = get_cookie_val(cookie_str, "SSID", ssid_str);
        if (ret == false)
        {
            // cookie中没有ssid, 返回错误: 没有ssid信息, 让客户端重新登录
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "找不到ssid信息, 请重新登录");
        }
        // 2. 在session管理中查找对应的会话信息
        session_ptr ssp = _sm.get_session_by_ssid(std::stol(ssid_str));
        if (ssp.get() == nullptr)
        {
            // 没有找到session, 则认为登录已经过期, 需要重新登录
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "登录过期, 请重新登录");
        }
        // 3. 从数据库中取出用户信息, 进行序列化发送给客户端
        uint64_t uid = ssp->get_user();
        Json::Value user_info;
        ret = _ut.select_by_id(uid, user_info);
        if (ret == false)
        {
            // 获取用户信息失败, 返回错误: 找不到用户信息
            return http_resp(conn, false, websocketpp::http::status_code::bad_request, "找不到用户信息, 请重新登录");
        }
        std::string body;
        json_util::serialize(user_info, body);
        conn->set_body(body);
        conn->append_header("Contect-Type", "application/json");
        conn->set_status(websocketpp::http::status_code::ok);
        // 4. 刷新session的过期时间
        _sm.set_session_expire_time(ssp->ssid(), SESSION_TIMEOUT);
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