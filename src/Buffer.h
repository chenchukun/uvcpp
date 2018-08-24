#ifndef LIBUVCPP_BUFFER_H
#define LIBUVCPP_BUFFER_H

#include "utility.h"
#include <string>
#include <assert.h>
#include <cstdlib>
#include <list>
#include <cstring>
#include <arpa/inet.h>
#include <uv.h>
#include <vector>

NAMESPACE_START

class BufferOutputStream;

class BufferInputStream;

class DataBlock
{
public:
    DataBlock(size_t len=1024);

    DataBlock(DataBlock &&data)
        : buffer_(std::move(data.buffer_)),
          read_(data.read_),
          write_(data.write_)
    {
    }

    DataBlock(const DataBlock &data)
        : buffer_(data.buffer_),
          read_(data.read_),
          write_(data.write_)
    {
    }

    DataBlock& operator=(const DataBlock &data) {
        buffer_ = data.buffer_;
        read_ = data.read_;
        write_ = data.write_;
        return *this;
    }

    void swap(DataBlock &data) {
        buffer_.swap(data.buffer_);
        std::swap(read_, data.read_);
        std::swap(write_, data.write_);
    }

    size_t readPosition() const {
        return read_;
    }

    size_t writePosition() const {
        return write_;
    }

    size_t capacity() const {
        return buffer_.size();
    }

    size_t readableBytes() const {
        return write_ - read_;
    }

    size_t writeableBytes() const {
        return buffer_.size() - write_;
    }

    size_t prependableBytes() const {
        return read_;
    }

    size_t totalSpace() const {
        return writeableBytes() + prependableBytes();
    }

    char* peek() {
        return &buffer_[0];
    }

    const char* peek() const {
        return &buffer_[0];
    }

    void setReadPosition(size_t pos) {
        assert(pos <= write_);
        read_ = pos;
        if (read_ == write_ && read_ != capacity()) {
            read_ = write_ = 0;
        }
    }

    void setWritePosition(size_t pos) {
        assert(pos >= read_ && pos <= capacity());
        write_ = pos;
        if (read_ == write_ && read_ != capacity()) {
            read_ = write_ = 0;
        }
    }

    void realign();

private:
    std::vector<char> buffer_;

    size_t read_;

    size_t write_;
};


class Buffer
{
public:
    friend class BufferOutputStream;

    friend class BufferInputStream;

    Buffer();

    Buffer(const Buffer &buffer);

    Buffer(Buffer &&buffer);

    Buffer& operator=(const Buffer &buffer);

    void swap(Buffer &buffer);

    size_t readableBytes() const {
        return readableBytes_;
    }

    size_t capacity() const {
        return capacity_;
    }

    void append(const void *ptr, size_t len);

    void append(const std::string &str) {
        append(str.c_str(), str.size());
    }

    void prepend(const void *ptr, size_t len);

    void prepend(const std::string &str) {
        prepend(str.c_str(), str.size());
    }

    std::string toString(size_t len) const;

    std::string toString() const;

    std::string retrieveAsString(size_t len);

    std::string retrieveCStyleString();

    std::string retrieveAllAsString();

    void discard(size_t len);

    void discardAll();

    void appendInt8(int8_t val) {
        append(&val, sizeof(val));
    }

    void appendInt16(int16_t val) {
        int16_t nval = htons(val);
        append(&nval, sizeof(val));
    }

    void appendInt32(int32_t val) {
        int32_t nval = htonl(val);
        append(&nval, sizeof(val));
    }

    void appendInt64(int64_t val) {
        int64_t nval = htonll(val);
        append(&nval, sizeof(val));
    }

    int8_t readInt8() {
        int8_t val = peekInt8();
        discard(sizeof(int8_t));
        return val;
    }

    int8_t peekInt8() const {
        std::string str = toString(sizeof(int8_t));
        const int8_t *val = reinterpret_cast<const int8_t*>(str.data());
        return *val;
    }

    int16_t readInt16() {
        int16_t val = peekInt16();
        discard(sizeof(int16_t));
        return val;
    }

    int16_t peekInt16() const {
        std::string str = toString(sizeof(int16_t));
        const int16_t *val = reinterpret_cast<const int16_t*>(str.data());
        return ntohs(*val);
    }

    int32_t readInt32() {
        int32_t val = peekInt32();
        discard(sizeof(int32_t));
        return val;
    }

    int32_t peekInt32() const {
        std::string str = toString(sizeof(int32_t));
        const int32_t *val = reinterpret_cast<const int32_t*>(str.data());
        return ntohl(*val);
    }

    int64_t readInt64() {
        int64_t val = peekInt64();
        discard(sizeof(int64_t));
        return val;
    }

    int64_t peekInt64() const {
        std::string str = toString(sizeof(int64_t));
        const int64_t *val = reinterpret_cast<const int64_t*>(str.data());
        return ntohll(*val);
    }

    void all(std::vector<std::pair<char*, size_t> > &bufs) const;

    void next(char* &data, size_t &len);

    void extend(size_t len);

    void unwrite(size_t len);

    void unread(size_t len);

    void debug() const;

private:
    std::list<DataBlock> list_;

    std::list<DataBlock>::iterator firstWithData_;

    std::list<DataBlock>::iterator lastWithData_;

    size_t readableBytes_;

    size_t capacity_;
};


NAMESPACE_END

#endif //LIBUVCPP_BUFFER_H
