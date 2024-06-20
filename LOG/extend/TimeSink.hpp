#ifndef __M_USERDEFSINK_H__
#define __M_USERDEFSINK_H__

/*  扩展一个以时间作为日志文件滚动切换类型的日志落地模块
    1. 以时间进行文件滚动, 实际上是以时间段进行滚动
    2. 实现思想: 以当前系统时间(时间戳), 整除时间段大小(1, 60, 3600, ...), 可以得到当前时间段编号
    3. 每次写日志都计算一下当前的时间段编号, 判断和上一次的编号是否一致, 不一致则新建一个日志文件
*/

#include "../logs/Sink.hpp"

enum class TimeGap
{
    GAP_SECOND,
    GAP_MINUTE,
    GAP_HOUR,
    GAY_DAY
};

class RollByTimeSink : public tjq::LogSink
{
public:
    // 构造时传入文件名, 并打开文件, 将操作句柄管理起来
    RollByTimeSink(const std::string &basename, TimeGap gap_type)
        : _basename(basename)
    {
        switch (gap_type)
        {
        case TimeGap::GAP_SECOND:
            _gap_size = 1;
            break;
        case TimeGap::GAP_MINUTE:
            _gap_size = 60;
            break;
        case TimeGap::GAP_HOUR:
            _gap_size = 3600;
            break;
        case TimeGap::GAY_DAY:
            _gap_size = 3600 * 24;
            break;
        default:
            std::cerr << "RollByTimeSink: 无效的时间间隔类型" << std::endl;
        }
        _cur_gap = getCurGap(); // 获取当前时间段
        std::string filename = createNewFile();
        tjq::tool::File::createDirectory(tjq::tool::File::path(filename));
        _ofs.open(filename, std::ios::binary | std::ios::app);
        assert(_ofs.is_open());
    }

    // 将日志消息写入到指定文件, 判断当前时间是否是当前文件的时间段, 不是则切换文件
    void log(const char *data, size_t len) override
    {
        size_t new_cur = getCurGap();
        if (new_cur != _cur_gap)
        {
            _cur_gap = new_cur; // 更新_cur_gap
            _ofs.close();
            std::string filename = createNewFile();
            _ofs.open(filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }
        _ofs.write(data, len);
        assert(_ofs.good());
    }

private:
    // 获取当前时间段
    size_t getCurGap()
    {
        time_t now = tjq::tool::Date::now();
        return now / _gap_size;
    }

    std::string createNewFile()
    {
        time_t t = tjq::tool::Date::now();
        struct tm lt;
        localtime_r(&t, &lt);
        std::stringstream filename;
        filename << _basename;
        filename << lt.tm_year + 1900;
        filename << lt.tm_mon + 1;
        filename << lt.tm_mday;
        filename << lt.tm_hour;
        filename << lt.tm_min;
        filename << lt.tm_sec;
        filename << ".log";
        return filename.str();
    }

private:
    std::string _basename;
    std::ofstream _ofs;
    size_t _cur_gap;  // 当前是第几个时间段
    size_t _gap_size; // 时间段的大小
};

#endif