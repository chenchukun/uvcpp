#ifndef LIBUVCPP_BUFFER_H
#define LIBUVCPP_BUFFER_H

#include "utility.h"
#include <string>
#include <assert.h>
#include <cstdlib>
#include <deque>
#include <cstring>
#include <arpa/inet.h>
#include <uv.h>
#include <vector>

NAMESPACE_START

struct Block
{
public:
    Block(size_t len)
        : start_(0),
          end_(0),
          cap_(len)
    {
        assert(len > 0);
        buf_ = static_cast<char*>(malloc(sizeof(char) * len));
        assert(buf_ != NULL);
    }

    Block(void *ptr, size_t len)
        : start_(0),
          end_(len),
          cap_(len)
    {
        buf_ = static_cast<char*>(ptr);
    }

    ~Block() {

    }

    Block(const Block &block) {
        start_ = block.start_;
        end_ = block.end_;
        cap_ = block.cap_;
        buf_ = static_cast<char*>(malloc(block.capacity()));
        assert(buf_ != NULL);
        memcpy(buf_ + start_, block.peek(), block.readableBytes());
    }

    Block& operator=(const Block &block) {
        start_ = block.start_;
        end_ = block.end_;
        cap_ = block.cap_;
        if (buf_ != NULL) {
            free(buf_);
        }
        buf_ = static_cast<char*>(malloc(block.capacity()));
        assert(buf_ != NULL);
        memcpy(buf_ + start_, block.peek(), block.readableBytes());
        return *this;
    }

    Block(Block &&block) {
        start_ = block.start_;
        end_ = block.end_;
        cap_ = block.cap_;
        block.start_ = block.end_ = block.cap_ = 0;
        buf_ = block.buf_;
        block.buf_ = NULL;
    }

    size_t readPosition() const {
        return start_;
    }

    size_t writePosition() const {
        return end_;
    }

    size_t capacity() const {
        return cap_;
    }

    size_t prependSpaceBytes() const {
        return start_;
    }

    size_t writableBytes() const {
        return cap_ - end_;
    }

    size_t readableBytes() const {
        return end_ - start_;
    }

    void setReadPosition(size_t pos) {
        assert(pos <= end_);
        start_ = pos;
        if (start_ == end_) {
            start_ = end_ = 0;
        }
    }

    void setWritePosition(size_t pos) {
        assert(pos <= cap_ && pos >= start_);
        end_ = pos;
        if (start_ == end_) {
            start_ = end_ = 0;
        }
    }

    char* peek() {
        return buf_ + start_;
    }

    const char* peek() const {
        return buf_ + start_;
    }

    char* head() {
        return buf_;
    }

    const char* head() const {
        return buf_;
    }

    bool empty() const {
        return start_ == end_;
    }

    bool full() const {
        return start_ == 0 && end_ == cap_;
    }

private:
    char *buf_;

    size_t start_, end_, cap_;
};

class Buffer
{
public:
    const static size_t initSize = 1024;

    explicit Buffer(size_t size=initSize)
    {
        blocks_.emplace_back(size);
    }

    Buffer(const Buffer &buffer);

    Buffer(Buffer &&buffer);

    size_t readableBytes() const;

    size_t writableBytes() const;

    size_t prependSpaceBytes() const;

    void ensureWritableBytes(size_t size) {
        if (writableBytes() < size) {
            size_t bytes = size - writableBytes();
            bytes += bytes<1024? bytes: 1024;
            blocks_.emplace_back(bytes);
        }
    }

    void ensurePrependBytes(size_t size) {
        if (prependSpaceBytes() < size) {
            size_t bytes = size - prependSpaceBytes();
            blocks_.emplace_front(bytes);
        }
    }

    int8_t peekInt8() const {
        std::string str = peekAsString(1);
        return *reinterpret_cast<const int8_t*>(str.data());
    }

    int8_t readInt8() {
        std::string str = retrieveAsString(1);
        return *reinterpret_cast<const int8_t*>(str.data());
    }

    void appendInt8(int8_t val) {
        append(&val, sizeof(val));
    }

    void prependInt8(int8_t val) {
        prepend(&val, sizeof(val));
    }

    int16_t peekInt16() const {
        std::string str = peekAsString(2);
        const int16_t *p = reinterpret_cast<const int16_t*>(str.data());
        return ntohs(*p);
    }

    int16_t readInt16() {
        std::string str = retrieveAsString(2);
        const int16_t *p = reinterpret_cast<const int16_t*>(str.data());
        return ntohs(*p);
    }

    void appendInt16(int16_t val) {
        int16_t nval = htons(val);
        append(&nval, sizeof(nval));
    }

    void prependInt16(int16_t val) {
        int16_t nval = htons(val);
        prepend(&nval, sizeof(nval));
    }

    int32_t peekInt32() const {
        std::string str = peekAsString(4);
        const int32_t *p = reinterpret_cast<const int32_t*>(str.data());
        return ntohl(*p);
    }

    int32_t readInt32() {
        std::string str = retrieveAsString(4);
        const int32_t *p = reinterpret_cast<const int32_t*>(str.data());
        return ntohl(*p);
    }

    void appendInt32(int32_t val) {
        int32_t nval = htonl(val);
        append(&nval, sizeof(nval));
    }

    void prependInt32(int32_t val) {
        int32_t nval = htonl(val);
        prepend(&nval, sizeof(nval));
    }

    int64_t peekInt64() const {
        std::string str = peekAsString(8);
        const int64_t *p = reinterpret_cast<const int64_t*>(str.data());
        return ntohll(*p);
    }

    int64_t readInt64() {
        std::string str = retrieveAsString(8);
        const int64_t *p = reinterpret_cast<const int64_t*>(str.data());
        return ntohll(*p);
    }

    void appendInt64(int64_t val) {
        int64_t nval = htonll(val);
        append(&nval, sizeof(nval));
    }

    void prependInt64(int64_t val) {
        int64_t nval = htonll(val);
        prepend(&nval, sizeof(nval));
    }

    void discard(size_t size);

    void discardAll() {
        blocks_.erase(blocks_.begin(), blocks_.end());
    }

    std::string retrieveAllAsString() {
        return std::move(retrieveAsString(readableBytes()));
    }

    std::string retrieveAsString(size_t size);

    std::string peekAsString(size_t size) const;

    void append(const void *ptr, size_t len);

    void prepend(const void *ptr, size_t len);

    void append(const std::string &str) {
        append(str.c_str(), str.size());
    }

    void prepend(const std::string &str) {
        prepend(str.c_str(), str.size());
    }

    void initUVBuffer(std::vector<uv_buf_t> &bufs) const;

private:
    std::pair<std::deque<Block>::iterator, size_t> appendPoint(size_t size);

    std::pair<std::deque<Block>::iterator, size_t> prependPoint(size_t size);

private:
    std::deque<Block> blocks_;
};

NAMESPACE_END

#endif //LIBUVCPP_BUFFER_H
