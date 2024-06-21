/*日志系统使用示例*/

#include <unistd.h>
#include "../logs/Log.h"
#include "../extend/TimeSink.hpp"

void build(const std::string &name)
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::GlobalLoggerBuilder());
    builder->buildLoggerName(name);
    builder->buildLoggerLevel(tjq::LogLevel::value::INFO);
    builder->buildFormatter("[%c][%f:%l][%p]%m%n");
    builder->buildLoggerType(tjq::LoggerType::LOGGER_SYNC);
    builder->buildSink<tjq::StdoutSink>();
    builder->buildSink<tjq::FileSink>("./logfile/sync.log");
    builder->buildSink<tjq::RollBySizeSink>("./logfile/roll-sync-by-size-", 1024 * 1024);
    builder->build();
}

void write(const std::string &name)
{
    INFO("%s", "测试开始...");

    tjq::Logger::ptr logger = tjq::LoggerManager::getInstance().getLogger(name);
    logger->debug("%s", "debug测试");
    logger->info("%s", "info测试");
    logger->warn("%s", "warn测试");
    logger->error("%s", "error测试");
    logger->fatal("%s", "fatal测试");

    INFO("%s", "测试结束...");
}

void testMySink(const std::string &name)
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::GlobalLoggerBuilder());
    builder->buildLoggerName(name);
    builder->buildLoggerType(tjq::LoggerType::LOGGER_ASYNC);
    builder->buildSink<tjq::StdoutSink>();
    builder->buildSink<ext::RollByTimeSink>("./logfile/roll-async-by-time-", ext::TimeGap::GAP_MINUTE);
    builder->build();

    tjq::Logger::ptr logger = tjq::LoggerManager::getInstance().getLogger(name);
    size_t time = tjq::tool::Date::now();
    while (tjq::tool::Date::now() < time + 120)
    {
        logger->fatal("%s-time:%d", "测试", tjq::tool::Date::now());
        usleep(1000);
    }
}

int main()
{
    /*---普通示例---*/
    std::string logger_name = "sync_logger";
    build(logger_name);
    write(logger_name);

    /*---扩展示例---*/
    // testMySink("time_async_logger");

    return 0;
}