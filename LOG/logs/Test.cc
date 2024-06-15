#include <thread>
#include "Tool.hpp"
#include "Level.hpp"
#include "Message.hpp"
#include "Format.hpp"

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

int main()
{
    // testThread();
    testFormat();

    return 0;
}