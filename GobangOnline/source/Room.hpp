#ifndef __G_ROOM_H__
#define __G_ROOM_H__

#include "Util.hpp"
#include "DB.hpp"
#include "Online.hpp"

#define BOARD_ROW 15
#define BOARD_COL 15
#define CHESS_WHITE 1
#define CHESS_BLACK 2

typedef enum
{
    GAME_START,
    GAME_OVER
} room_statu;

class room
{
public:
    room(uint64_t room_id, user_table *tb_user, online_manager *online_user)
        : _room_id(room_id),
          _statu(GAME_START),
          _player_count(0),
          _tb_user(tb_user),
          _online_user(online_user),
          _board(BOARD_ROW, std::vector<int>(BOARD_COL, 0))
    {
        DEBUG("%lu: 房间创建成功!", _room_id);
    }
    ~room()
    {
        DEBUG("%lu: 房间销毁成功!", _room_id);
    }

    uint64_t id()
    {
        return _room_id;
    }
    room_statu statu()
    {
        return _statu;
    }
    int player_count()
    {
        return _player_count;
    }
    void add_white_user(uint64_t uid)
    {
        _white_id = uid;
        _player_count++;
    }
    void add_black_user(uint64_t uid)
    {
        _black_id = uid;
        _player_count++;
    }
    uint64_t get_white_user()
    {
        return _white_id;
    }
    uint64_t get_black_user()
    {
        return _black_id;
    }

    // 处理下棋动作
    Json::Value handle_chess(Json::Value &req)
    {
        Json::Value json_resp;
        // 1. 当前请求的房间号是否与当前房间的房间号匹配
        uint64_t room_id = req["room_id"].asUInt64();
        if (room_id != _room_id)
        {
            json_resp["optype"] = "put_chess";
            json_resp["result"] = false;
            json_resp["reason"] = "房间号不匹配!";
            return json_resp;
        }
        // 2. 判断房间中两个玩家是否都在线, 任意一个不在线, 就是另一方胜利
        int chess_row = req["row"].asInt();
        int chess_col = req["col"].asInt();
        uint64_t cur_uid = req["uid"].asInt64();
        if (_online_user->is_in_game_room(_white_id) == false)
        {
            json_resp["optype"] = "put_chess";
            json_resp["result"] = true;
            json_resp["reason"] = "对方已离开房间, 你赢了";
            json_resp["room_id"] = (Json::UInt64)_room_id;
            json_resp["uid"] = (Json::UInt64)cur_uid;
            json_resp["row"] = chess_row;
            json_resp["col"] = chess_col;
            json_resp["winner"] = (Json::UInt64)_black_id;
            uint64_t loser_id = _white_id;
            _tb_user->win(_black_id);
            _tb_user->lose(_white_id);
            return json_resp;
        }
        if (_online_user->is_in_game_room(_black_id) == false)
        {
            json_resp["optype"] = "put_chess";
            json_resp["result"] = true;
            json_resp["reason"] = "对方已离开房间, 你赢了";
            json_resp["room_id"] = (Json::UInt64)_room_id;
            json_resp["uid"] = (Json::UInt64)cur_uid;
            json_resp["row"] = chess_row;
            json_resp["col"] = chess_col;
            json_resp["winner"] = (Json::UInt64)_white_id;
            uint64_t loser_id = _black_id;
            _tb_user->win(_white_id);
            _tb_user->lose(_black_id);
            return json_resp;
        }
        // 3. 判断走棋位置, 判断当前走棋是否合理(位置是否已经被占用)
        if (_board[chess_row][chess_col] != 0)
        {
            json_resp["optype"] = "put_chess";
            json_resp["result"] = false;
            json_resp["reason"] = "当前位置已经有了其他棋子";
            return json_resp;
        }
        int cur_color = cur_uid == _white_id ? CHESS_WHITE : CHESS_BLACK;
        _board[chess_row][chess_col] = cur_color;
        // 4. 判断是否有玩家胜利
        uint64_t winner_id = check_win(chess_row, chess_col, cur_color);
        if (winner_id != 0)
        {
            // 更新数据库用户信息
            uint64_t loser_id = winner_id == _white_id ? _black_id : _white_id;
            _tb_user->win(winner_id);
            _tb_user->lose(loser_id);
        }
        json_resp["optype"] = "put_chess";
        json_resp["result"] = true;
        json_resp["reason"] = "五星连珠, 你赢了!";
        json_resp["room_id"] = (Json::UInt64)_room_id;
        json_resp["uid"] = (Json::UInt64)cur_uid;
        json_resp["row"] = chess_row;
        json_resp["col"] = chess_col;
        json_resp["winner"] = (Json::UInt64)winner_id;
        return json_resp;
    }

    // 处理聊天动作
    Json::Value handle_chat(Json::Value &req);
    // 处理玩家退出房间动作
    void handle_exit(uint64_t uid);
    // 总的请求处理函数, 在函数内部, 区分请求类型, 根据不同的请求调用不同的处理函数, 得到响应进行广播
    void handle_request(Json::Value &req);
    // 将指定的信息广播给房间中所有玩家
    void broadcast(Json::Value &req);

private:
    bool five(int row, int col, int row_off, int col_off, int color)
    {
        // row和col是下棋位置, row_off和col_off是偏移量
        int count = 1;
        int search_row = row + row_off;
        int search_col = col + col_off;
        while (search_row >= 0 && search_row < BOARD_ROW &&
               search_col >= 0 && search_col < BOARD_COL &&
               _board[search_row][search_col] == color)
        {
            // 同色棋子数量++
            count++;
            // 检索位置继续向后偏移
            search_row += row_off;
            search_col += col_off;
        }
        search_row = row - row_off;
        search_col = col - col_off;
        while (search_row >= 0 && search_row < BOARD_ROW &&
               search_col >= 0 && search_col < BOARD_COL &&
               _board[search_row][search_col] == color)
        {
            // 同色棋子数量++
            count++;
            // 检索位置继续向后偏移
            search_row -= row_off;
            search_col -= col_off;
        }
        return (count >= 5);
    }

    // 返回胜利玩家的ID, 没有胜利者返回0
    uint64_t check_win(int row, int col, int color)
    {
        if (five(row, col, 0, 1, color) ||
            five(row, col, 1, 0, color) ||
            five(row, col, -1, 1, color) ||
            five(row, col, -1, -1, color))
        {
            return color == CHESS_WHITE ? _white_id : _black_id;
        }
        return 0;
    }

private:
    uint64_t _room_id;
    room_statu _statu;
    int _player_count;
    uint64_t _white_id;
    uint64_t _black_id;
    user_table *_tb_user;
    online_manager *_online_user;
    std::vector<std::vector<int>> _board;
};

#endif