#ifndef __M_BUFFER_H__
#define __M_BUFFER_H__

/*实现异步日志缓冲区*/

#include <vector>
#include "Tool.hpp"

namespace tjq
{
#define DEFAULT_BUFFER_SIZE (10 * 1024 * 1024)
#define THRESHOLD_BUFFER_SIZE (80 * 1024 * 1024)
#define INCREMENT_BUFFER_SIZE (10 * 1024 * 1024)

    class Buffer
    {
    public:
        Buffer()
            : _buffer(DEFAULT_BUFFER_SIZE),
              _writer_idx(0),
              _reader_idx(0)
        {
        }

        // 向缓冲区写入数据
        void push(const char *data, size_t len)
        {
            // 缓冲区剩余空间不够的情况
            // 1. 固定大小, 直接返回
            if (len > writeAbleSize())
                return;
            // 2. 动态空间, 用于极限性能测试 - 扩容
            ensureEnoughSize(len);
            // 将数据拷贝进缓冲区
            std::copy(data, data + len, &_buffer[_writer_idx]);
            // 将当前写入位置向后偏移
            moveWriter(len);
        }

        // 返回可读数据的起始地址
        const char *begin();
        // 返回可读数据的长度
        size_t readAbleSize();
        size_t writeAbleSize();
        void moveReader(size_t len);
        // 重置读写位置, 初始化缓冲区
        void reset();
        // 对Buffer实现交换操作
        void swap(const Buffer &buffer);
        // 判断缓冲区是否为空
        bool empty();

    private:
        // 对空间进行扩容
        void ensureEnoughSize(size_t len)
        {
            if (len < writeAbleSize())
            {
                return; // 不需要扩容
            }
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_BUFFER_SIZE)
            {
                new_size = _buffer.size() * 2; // 小于阈值则翻倍增长
            }
            else
            {
                new_size = _buffer.size() + INCREMENT_BUFFER_SIZE; // 否则线性增长
            }
            _buffer.resize(new_size);
        }

        // 对读写指针进行向后偏移操作
        void moveWriter(size_t len);

    private:
        std::vector<char> _buffer;
        size_t _reader_idx; // 当前可读数据的指针 - 本质是下标
        size_t _writer_idx; // 当前可写数据的指针
    };
}

#endif