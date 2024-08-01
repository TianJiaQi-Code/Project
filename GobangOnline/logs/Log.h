#ifndef __M_LOG_H__
#define __M_LOG_H__

/*  1. 提供获取指定日志器的全局接口(避免用户自己操作单例对象)
    2. 使用宏函数对日志器的接口进行代理(代理模式)
    3. 提供宏函数, 直接通过默认日志器进行日志的标准输出打印(不用获取日志器了)
*/

#include "Logger.hpp"

// 使用宏函数对日志器的接口进行代理(代理模式)
#define log_debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

// 提供宏函数, 直接通过默认日志器进行日志的标准输出打印(不用获取日志器了)
#define DEBUG(fmt, ...) tjq::rootLogger()->log_debug(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) tjq::rootLogger()->log_info(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) tjq::rootLogger()->log_warn(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) tjq::rootLogger()->log_error(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) tjq::rootLogger()->log_fatal(fmt, ##__VA_ARGS__)

namespace tjq
{
    // 提供获取指定日志器的全局接口(避免用户自己操作单例对象)
    Logger::ptr getLogger(const std::string &name)
    {
        return tjq::LoggerManager::getInstance().getLogger(name);
    }
    Logger::ptr rootLogger()
    {
        return tjq::LoggerManager::getInstance().rootLogger();
    }
}

#endif