#ifndef __M_LEVEL_H__
#define __M_LEVEL_H__

/*  日志等级类的实现:
    1. 定义枚举类, 枚举出日志等级
    2. 提供转换接口: 将枚举转换为对应字符串
*/

namespace tjq
{
    class LogLevel
    {
    public:
        enum class value
        {
            UNKNOW = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            OFF
        };

        static const char *toString(LogLevel::value level)
        {
            switch (level)
            {
            case LogLevel::value::DEBUG:
                return "DEBUG";
            case LogLevel::value::INFO:
                return "INFO";
            case LogLevel::value::WARN:
                return "WARN";
            case LogLevel::value::ERROR:
                return "ERROR";
            case LogLevel::value::FATAL:
                return "FATAL";
            case LogLevel::value::OFF:
                return "OFF";
            }
            return "UNKNOW";
        }
    };
}

#endif