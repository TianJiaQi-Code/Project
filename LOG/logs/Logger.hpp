#ifndef __M_LOGGER_H__
#define __M_LOGGER_H__

/*  完成日志器模块
    1. 抽象日志器基类
    2. 派生出不同的子类 (同步日志器类 & 异步日志器类)
*/

#include <mutex>
#include <atomic>
#include <cstdarg>
#include "Level.hpp"
#include "Format.hpp"
#include "Sink.hpp"

namespace tjq
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, LogLevel::value level, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sinks)
            : _logger_name(logger_name),
              _limit_level(level),
              _formatter(formatter),
              _sinks(sinks.begin(), sinks.end())
        {
        }

        // 完成构造日志消息对象过程并进行格式化, 得到格式化后的日志消息字符串, 然后进行落地输出
        void debug(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 通过传入的参数构造出一个日志消息对象, 进行日志的格式化, 最终落地
            // 1. 判断当前的日志是否达到了输出等级
            if (LogLevel::value::DEBUG < _limit_level)
            {
                return;
            }
            // 2. 对fmt格式化字符和不定参进行字符串组织, 得到日志消息字符串
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cerr << "Logger.hpp::Logger::debug: vasprintf failed!" << std::endl;
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::DEBUG, file, line, res);
            free(res);
        }
        void info(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            if (LogLevel::value::INFO < _limit_level)
            {
                return;
            }
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cerr << "Logger.hpp::Logger::info: vasprintf failed!" << std::endl;
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::INFO, file, line, res);
            free(res);
        }
        void warn(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            if (LogLevel::value::WARN < _limit_level)
            {
                return;
            }
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cerr << "Logger.hpp::Logger::warn: vasprintf failed!" << std::endl;
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::WARN, file, line, res);
            free(res);
        }
        void error(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            if (LogLevel::value::ERROR < _limit_level)
            {
                return;
            }
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cerr << "Logger.hpp::Logger::error: vasprintf failed!" << std::endl;
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::ERROR, file, line, res);
            free(res);
        }
        void fatal(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            if (LogLevel::value::FATAL < _limit_level)
            {
                return;
            }
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cerr << "Logger.hpp::Logger::fatal: vasprintf failed!" << std::endl;
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::FATAL, file, line, res);
            free(res);
        }

    protected:
        void serialize(LogLevel::value level, const std::string &file, size_t line, char *str)
        {
            // 3. 构造LogMessage对象
            LogMessage msg(level, line, file, _logger_name, str);
            // 4. 通过格式化工具对LogMessage进行格式化, 得到格式化后的日志字符串
            std::stringstream ss;
            _formatter->format(ss, msg);
            // 5. 进行日志落地
            log(ss.str().c_str(), ss.str().size());
        }

        // 抽象接口完成实际的落地输出 - 不同的日志器会有不同的实际落地方式
        virtual void log(const char *data, size_t len) = 0;

    protected:
        std::mutex _mutex;
        std::string _logger_name;
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string &logger_name, LogLevel::value level, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sinks)
            : Logger(logger_name, level, formatter, sinks)
        {
        }

    protected:
        // 同步日志器, 是将日志直接通过落地模块句柄进行日志落地
        void log(const char *data, size_t len)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_sinks.empty())
                return;
            for (auto &sink : _sinks)
            {
                sink->log(data, len);
            }
        }
    };

    /*  使用建造者模式来建造日志器, 而不要让用户直接去构造日志器, 简化用户的使用复杂度
        1. 抽象一个日志器建造者类 (完成日志器对象所需零部件的构建 & 日志器的构建)
            1) 设置日志器类型
            2) 将不同类型日志器的创建放到同一个日志器建造者类中完成
    */
    enum class LoggerType
    {
        LOGGER_SYNC,
        LOGGER_ASYNC
    };
    class LoggerBuilder
    {
    public:
        LoggerBuilder()
            : _logger_type(LoggerType::LOGGER_SYNC),
              _limit_level(LogLevel::value::DEBUG)
        {
        }
        void buildLoggerType(LoggerType type)
        {
            _logger_type = type;
        }
        void buildLoggerName(const std::string &name)
        {
            _logger_name = name;
        }
        void buildLoggerLevel(LogLevel::value level)
        {
            _limit_level = level;
        }
        void buildFormatter(const std::string &pattern)
        {
            _formatter = std::make_shared<Formatter>(pattern);
        }
        template <typename SinkType, typename... Args>
        void buildSink(Args &&...args)
        {
            LogSink::ptr psink = SinkFactory::create<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(psink);
        }
        virtual Logger::ptr build() = 0;

    protected:
        LoggerType _logger_type;
        std::string _logger_name;
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };
    //  2. 派生出具体的建造者类 - 局部日志器建造者 &全局日志器建造者(后面添加了全局单例管理器之后, 将日志器添加全局管理)
    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(_logger_name.empty() == false); // 必须有日志器名称
            if (_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            if (_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            if (_logger_type == LoggerType::LOGGER_ASYNC)
            {
            }
            return std::make_shared<SyncLogger>(_logger_name, _limit_level, _formatter, _sinks);
        }
    };
}

#endif