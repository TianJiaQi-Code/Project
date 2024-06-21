#include "../logs/Log.h"

void performanceTest(const std::string &logger_name, size_t thr_count, size_t msg_count, size_t msg_len)
{
    // 1. 获取日志器
    tjq::Logger::ptr logger = tjq::getLogger(logger_name);
    if (logger.get() == nullptr)
    {
        return;
    }
    std::cout << "测试日志: " << msg_count << " 条, 总大小: " << (msg_count * msg_len) / 1024 << " KB" << std::endl;
    // 2. 组织指定长度的日志消息
    std::string msg(msg_len - 1, 'X'); // 少一个字节, 是为了给末尾添加换行
    // 3. 创建指定数量的进程
    std::vector<std::thread> threads;
    std::vector<double> cost_arry(thr_count);
    size_t msg_per_thr = msg_count / thr_count; //(总日志数量/线程数量)就是每个线程要输出的日志数量
    for (int i = 0; i < thr_count; i++)
    {
        threads.emplace_back([&, i]()
                             {
                                 // 4. 线程函数内部开始计时
                                 auto start = std::chrono::high_resolution_clock::now();
                                 // 5. 开始循环写日志
                                 for (int j = 0; j < msg_per_thr; j++)
                                 {
                                     logger->fatal("%s", msg.c_str());
                                 }
                                 // 6. 线程函数内部结束计时
                                 auto end = std::chrono::high_resolution_clock::now();
                                 std::chrono::duration<double> cost = end - start;
                                 cost_arry[i] = cost.count();
                                 std::cout << "线程 " << i << " - 输出数量: " << msg_per_thr << " 条, 耗时: " << cost.count() << " s" << std::endl; });
    }
    for (int i = 0; i < thr_count; i++)
    {
        threads[i].join();
    }
    // 7. 计算总耗时: 在多线程中, 每个线程都会耗费时间, 但是线程是并发处理的, 因此耗时最高的那个线程所耗费的时间就是总时间
    double max_cost = cost_arry[0];
    for (int i = 0; i < thr_count; i++)
    {
        max_cost = max_cost < cost_arry[i] ? cost_arry[i] : max_cost;
    }
    size_t msg_per_sec = msg_count / max_cost;
    size_t size_per_sec = (msg_count * msg_len) / (max_cost * 1024);
    // 8. 进行输出打印
    std::cout << "总耗时: " << max_cost << " s" << std::endl;
    std::cout << "每秒输出日志数量: " << msg_per_sec << " 条" << std::endl;
    std::cout << "每秒输出日志大小: " << size_per_sec << " KB" << std::endl;
}

void syncPerf()
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::GlobalLoggerBuilder());
    builder->buildLoggerName("sync_logger");
    builder->buildFormatter("%m%n");
    builder->buildLoggerType(tjq::LoggerType::LOGGER_SYNC);
    builder->buildSink<tjq::FileSink>("./logfile/sync.log");
    builder->build();

    performanceTest("sync_logger", 5, 1000000, 100);
}

void asyncPerf()
{
    std::unique_ptr<tjq::LoggerBuilder> builder(new tjq::GlobalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildFormatter("%m%n");
    builder->buildLoggerType(tjq::LoggerType::LOGGER_ASYNC);
    builder->buildEnableUnSafeAsync(); // 开启非安全模式 - 主要是为了将实际落地时间排除在外
    builder->buildSink<tjq::FileSink>("./logfile/async.log");
    builder->build();

    performanceTest("async_logger", 5, 1000000, 100);
}

int main()
{
    syncPerf();
    // asyncPerf();

    return 0;
}