#ifndef __M_BUFFER_H__
#define __M_BUFFER_H__

/*实现异步日志缓冲区*/

#include <vector>
#include <cassert>
#include "Tool.hpp"

namespace tjq
{
#define DEFAULT_BUFFER_SIZE (1 * 1024 * 1024)
#define THRESHOLD_BUFFER_SIZE (8 * 1024 * 1024)
#define INCREMENT_BUFFER_SIZE (1 * 1024 * 1024)

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
            // 考虑空间不够则扩容
            ensureEnoughSize(len);
            // 将数据拷贝进缓冲区
            std::copy(data, data + len, &_buffer[_writer_idx]);
            // 将当前写入位置向后偏移
            moveWriter(len);
        }

        // 返回可读数据的起始地址
        const char *begin()
        {
            return &_buffer[_reader_idx];
        }

        // 对读指针进行向后偏移操作
        void moveReader(size_t len)
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }

        // 返回可读数据的长度
        size_t readAbleSize()
        {
            // 因为当前实现的缓冲区设计思想是双缓冲区, 处理完就交换, 所以不存在空间循环使用
            return (_writer_idx - _reader_idx);
        }

        // 返回剩余可写空间的大小
        size_t writeAbleSize()
        {
            // 对于扩容思路来说, 不存在可写空间大小, 因为总是可写
            // 因此这个接口仅仅针对固定大小缓冲区提供
            return (_buffer.size() - _writer_idx);
        }

        // 重置读写位置, 初始化缓冲区
        void reset()
        {
            _writer_idx = 0; // 缓冲区所有空间都是空闲的
            _reader_idx = 0; // 与_writer_idx相等表示没有数据可读
        }

        // 对Buffer实现交换操作
        void swap(Buffer &buffer)
        {
            _buffer.swap(buffer._buffer);
            std::swap(_reader_idx, buffer._reader_idx);
            std::swap(_writer_idx, buffer._writer_idx);
        }

        // 判断缓冲区是否为空
        bool empty()
        {
            return (_reader_idx == _writer_idx);
        }

    private:
        // 对空间进行扩容
        void ensureEnoughSize(size_t len)
        {
            if (len <= writeAbleSize())
            {
                return; // 不需要扩容
            }
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_BUFFER_SIZE)
            {
                new_size = _buffer.size() * 2 + len; // 小于阈值则翻倍增长
            }
            else
            {
                new_size = _buffer.size() + INCREMENT_BUFFER_SIZE + len; // 否则线性增长
            }
            _buffer.resize(new_size);
        }

        // 对写指针进行向后偏移操作
        void moveWriter(size_t len)
        {
            assert(len + _writer_idx <= _buffer.size());
            _writer_idx += len;
        }

    private:
        std::vector<char> _buffer;
        size_t _reader_idx; // 当前可读数据的指针 - 本质是下标
        size_t _writer_idx; // 当前可写数据的指针
    };
}

#endif