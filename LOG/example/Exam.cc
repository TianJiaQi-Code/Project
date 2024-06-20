/*日志系统使用示例*/

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

    builder->build();
}

int main()
{
    /*---普通测试---*/
    // std::string logger_name = "sync_logger";
    // build(logger_name);
    // write(logger_name);

    /*---扩展测试---*/
    testMySink("time_async_logger");

    return 0;
}