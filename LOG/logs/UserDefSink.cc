/*  扩展一个以时间作为日志文件滚动切换类型的日志落地模块
    1. 以时间进行文件滚动, 实际上是以时间段进行滚动
    2. 实现思想: 以当前系统时间, 取模时间段大小, 可以得到当前时间段是第几个时间段
    3. 每次以当前系统时间取模, 判断与当前文件的时间段是否一致, 不一致代表不是同一个时间段
*/

#include "Sink.hpp"

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
        }
        _cur_gap = tjq::tool::Date::now() % _gap_size; // 获取当前是第几个时间段
        std::string filename = createNewFile();
    }
    // 将日志消息写入到指定文件
    void log(const char *data, size_t len)
    {
        _ofs.write(data, len);
        assert(_ofs.good());
    }

private:
    std::string createNewFile()
    {
    }

private:
    std::string _basename;
    std::ofstream _ofs;
    size_t _cur_gap;  // 当前是第几个时间段
    size_t _gap_size; // 时间段的大小
};