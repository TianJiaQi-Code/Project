#include <thread>
#include <unistd.h>
#include "Tool.hpp"
#include "Level.hpp"
#include "Message.hpp"
#include "Format.hpp"
#include "Sink.hpp"
#include "UserDefSink.hpp"
#include "Logger.hpp"
#include "Buffer.hpp"

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

void testLoggerBuilder()
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::LocalLoggerBuilder());
    builder->buildLoggerName("sync_logger");
    builder->buildLoggerLevel(tjq::LogLevel::value::WARN);
    builder->buildFormatter("%m%n");
    builder->buildLoggerType(tjq::LoggerType::LOGGER_SYNC);
    builder->buildSink<tjq::StdoutSink>();
    builder->buildSink<tjq::FileSink>("./logfile/test.log");
    tjq::Logger::ptr logger = builder->build();

    logger->debug(__FILE__, __LINE__, "%s", "debug测试");
    logger->info(__FILE__, __LINE__, "%s", "info测试");
    logger->warn(__FILE__, __LINE__, "%s", "warn测试");
    logger->error(__FILE__, __LINE__, "%s", "error测试");
    logger->fatal(__FILE__, __LINE__, "%s", "fatal测试");

    size_t cursize = 0, count = 0;
    while (cursize < 10 * 1024 * 1024)
    {
        std::string msg = "测试日志: %d";
        logger->fatal(__FILE__, __LINE__, msg, count++);
        cursize += msg.size();
    }
}

void testBuffer()
{
    // 读取文件数据, 一点一点写入缓冲区, 最终将缓冲区数据写入文件, 判断生成的新文件与源文件是否一致
    std::ifstream ifs("./logfile/test.log", std::ios::binary);
    if (ifs.is_open() == false)
    {
        std::cerr << "open faild!" << std::endl;
        return;
    }
    ifs.seekg(0, std::ios::end); // 读写位置跳转到文件末尾
    size_t fsize = ifs.tellg();  // 获取当前读写位置相对于起始位置的偏移量
    ifs.seekg(0, std::ios::beg); // 重新跳转到起始位置

    std::string body;
    body.resize(fsize);
    ifs.read(&body[0], fsize);
    if (ifs.good() == false)
    {
        std::cerr << "read error" << std::endl;
        return;
    }
    ifs.close();

    tjq::Buffer buffer;
    for (int i = 0; i < body.size(); i++)
    {
        buffer.push(&body[i], 1);
    }
    std::ofstream ofs("./logfile/tmp.log", std::ios::binary);
    // ofs.write(buffer.begin(), buffer.readAbleSize());
    size_t rsize = buffer.readAbleSize();
    for (int i = 0; i < rsize; i++)
    {
        ofs.write(buffer.begin(), 1);
        buffer.moveReader(1);
    }
    ofs.close();
}

void testAsyncLogger()
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::LocalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(tjq::LogLevel::value::WARN);
    builder->buildFormatter("[%c]%m%n");
    builder->buildLoggerType(tjq::LoggerType::LOGGER_ASYNC);
    builder->buildEnableUnSafeAsync();
    builder->buildSink<tjq::StdoutSink>();
    builder->buildSink<tjq::FileSink>("./logfile/async.log");
    tjq::Logger::ptr logger = builder->build();

    logger->debug(__FILE__, __LINE__, "%s", "debug测试");
    logger->info(__FILE__, __LINE__, "%s", "info测试");
    logger->warn(__FILE__, __LINE__, "%s", "warn测试");
    logger->error(__FILE__, __LINE__, "%s", "error测试");
    logger->fatal(__FILE__, __LINE__, "%s", "fatal测试");

    size_t count = 0;
    while (count < 500000)
    {
        std::string msg = "测试日志: %d";
        logger->fatal(__FILE__, __LINE__, msg, count++);
    }
}

void writeLog()
{
    tjq::Logger::ptr logger = tjq::LoggerManager::getInstance().getLogger("async_logger");

    logger->debug(__FILE__, __LINE__, "%s", "debug测试");
    logger->info(__FILE__, __LINE__, "%s", "info测试");
    logger->warn(__FILE__, __LINE__, "%s", "warn测试");
    logger->error(__FILE__, __LINE__, "%s", "error测试");
    logger->fatal(__FILE__, __LINE__, "%s", "fatal测试");

    size_t count = 0;
    while (count < 500000)
    {
        std::string msg = "测试日志: %d";
        logger->fatal(__FILE__, __LINE__, msg, count++);
    }
}

void testGlobalLoggerBuilder()
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::GlobalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(tjq::LogLevel::value::WARN);
    builder->buildFormatter("[%c]%m%n");
    builder->buildLoggerType(tjq::LoggerType::LOGGER_ASYNC);
    builder->buildEnableUnSafeAsync();
    builder->buildSink<tjq::StdoutSink>();
    builder->buildSink<tjq::FileSink>("./logfile/async.log");
    builder->build();

    writeLog();
}

int main()
{
    testGlobalLoggerBuilder();
    // testAsyncLogger();
    // testBuffer();
    // testLoggerBuilder();
    // testLogger();
    // testUserDefSink();
    // testSink();
    // testThread();
    // testFormat();

    return 0;
}