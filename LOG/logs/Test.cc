#include <thread>
#include <unistd.h>
#include "Tool.hpp"
#include "Level.hpp"
#include "Message.hpp"
#include "Format.hpp"
#include "Sink.hpp"
#include "UserDefSink.hpp"
#include "Logger.hpp"

void testTool()
{
    std::cout << tjq::tool::Date::now() << std::endl;
    std::cout << tjq::tool::File::exists("./a.txt") << std::endl;
    std::cout << tjq::tool::File::exists("./Test.cc") << std::endl;
    std::cout << tjq::tool::File::path("./abc/def/b.txt") << std::endl;
    tjq::tool::File::createDirectory(tjq::tool::File::path("./abc/def/b.txt"));
}

void testLevel()
{
    std::cout << tjq::LogLevel::toString(tjq::LogLevel::value::ERROR) << std::endl;
    std::cout << tjq::LogLevel::toString(tjq::LogLevel::value::UNKNOW) << std::endl;
}

void testMessage()
{
}

void testFormat()
{
    tjq::LogMessage msg(tjq::LogLevel::value::INFO, 33, "main.c", "root", "格式化功能测试...");
    tjq::Formatter fmt("abc%%def[%d{%H:%M:%S}] %m%n%g");
    std::string str = fmt.format(msg);
    std::cout << str << std::endl;
}

void printThreadID()
{
    std::thread::id this_id = std::this_thread::get_id();
    std::cout << "Thread ID: " << this_id << std::endl;
}

void testThread()
{
    std::thread t1(printThreadID);
    std::thread t2(printThreadID);

    t1.join();
    t2.join();

    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
}

void testSink()
{
    tjq::LogMessage msg(tjq::LogLevel::value::INFO, 33, "main.c", "root", "格式化功能测试...");
    tjq::Formatter fmt;
    std::string str = fmt.format(msg);
    std::cout << str << std::endl;

    tjq::LogSink::ptr stdout_lsp = tjq::SinkFactory::create<tjq::StdoutSink>();
    tjq::LogSink::ptr file_lsp = tjq::SinkFactory::create<tjq::FileSink>("./logfile/test.log");
    tjq::LogSink::ptr roll_lsp = tjq::SinkFactory::create<tjq::RollBySizeSink>("./logfile/roll-", 1024 * 1024);

    stdout_lsp->log(str.c_str(), str.size());
    file_lsp->log(str.c_str(), str.size());
    size_t cursize = 0;
    size_t count = 0;
    while (cursize < 1024 * 1024 * 10)
    {
        std::string tmp = str + std::to_string(count++);
        roll_lsp->log(tmp.c_str(), tmp.size());
        cursize += tmp.size();
    }
}

void testUserDefSink()
{
    tjq::LogMessage msg(tjq::LogLevel::value::INFO, 33, "main.c", "root", "格式化功能测试...");
    tjq::Formatter fmt;
    std::string str = fmt.format(msg);

    tjq::LogSink::ptr time_lsp = tjq::SinkFactory::create<RollByTimeSink>("./logfile/roll-", TimeGap::GAP_MINUTE);
    // time_t old = tjq::tool::Date::now();
    // while (tjq::tool::Date::now() < old + 5)
    // {
    //     time_lsp->log(str.c_str(), str.size());
    //     usleep(1000);
    // }
    size_t time = 120;
    while (time--)
    {
        time_lsp->log(str.c_str(), str.size());
        sleep(1);
    }
}

void testLogger()
{
    tjq::Formatter::ptr fmt(new tjq::Formatter());

    tjq::LogSink::ptr stdout_lsp = tjq::SinkFactory::create<tjq::StdoutSink>();
    // tjq::LogSink::ptr file_lsp = tjq::SinkFactory::create<tjq::FileSink>("./logfile/test.log");
    // tjq::LogSink::ptr roll_lsp = tjq::SinkFactory::create<tjq::RollBySizeSink>("./logfile/roll-", 1024 * 1024);
    // std::vector<tjq::LogSink::ptr> sinks = {stdout_lsp, file_lsp, roll_lsp};
    // tjq::Logger::ptr logger(new tjq::SyncLogger("sync_logger", tjq::LogLevel::value::WARN, fmt, sinks));

    tjq::LogSink::ptr time_lsp = tjq::SinkFactory::create<RollByTimeSink>("./logfile/roll-", TimeGap::GAP_SECOND);
    std::vector<tjq::LogSink::ptr> sinks = {time_lsp, stdout_lsp};

    tjq::Logger::ptr logger(new tjq::SyncLogger("sync_logger", tjq::LogLevel::value::WARN, fmt, sinks));

    logger->debug(__FILE__, __LINE__, "%s", "debug测试");
    logger->info(__FILE__, __LINE__, "%s", "info测试");
    logger->warn(__FILE__, __LINE__, "%s", "warn测试");
    logger->error(__FILE__, __LINE__, "%s", "error测试");
    logger->fatal(__FILE__, __LINE__, "%s", "fatal测试");

    size_t cursize = 0, count = 0;
    while (cursize < 1024 * 1024)
    {
        std::string msg = "测试日志: %d";
        logger->fatal(__FILE__, __LINE__, msg, count++);
        cursize += msg.size();
    }
}

int main()
{
    testLogger();
    // testUserDefSink();
    // testSink();
    // testThread();
    // testFormat();

    return 0;
}