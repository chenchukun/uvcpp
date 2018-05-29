#ifndef LIBUVCPP_BUFFERSTREAM_H
#define LIBUVCPP_BUFFERSTREAM_H

#include "Buffer.h"
#include <google/protobuf/io/zero_copy_stream.h>

NAMESPACE_START

class BufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream
{
public:
    BufferOutputStream(Buffer *buffer)
        : buffer_(buffer),
          originalSize_(buffer_->readableBytes())
    {

    }

    virtual bool Next(void** data, int* size) {
        char **buf = reinterpret_cast<char**>(data);
        size_t len;
        buffer_->next(*buf, len);
        buffer_->extend(len);
        *size = len;
        return true;
    }

    virtual void BackUp(int count) {
        buffer_->unwrite(count);
    }

    virtual int64_t ByteCount() const {
        return buffer_->readableBytes() - originalSize_;
    }

private:
    Buffer *buffer_;

    size_t originalSize_;
};

class BufferInputStream : public google::protobuf::io::ZeroCopyInputStream
{
public:
    BufferInputStream(Buffer *buffer, size_t len)
        : buffer_(buffer),
          originalSize_(buffer_->readableBytes()),
          bytes_(len)
    {

    }

    virtual bool Next(const void** data, int* size) {
        if (bytes_ <= 0) {
            return false;
        }
        assert(buffer_->readableBytes() > 0);
        std::vector<std::pair<char*, size_t> > bufs;
        buffer_->all(bufs);
        *data = bufs[0].first;
        *size = bufs[0].second;
        bytes_ -= bufs[0].second;
        buffer_->firstWithData_++;
        buffer_->readableBytes_ -= *size;
        return true;
    }

    virtual void BackUp(int count) {
        buffer_->firstWithData_--;
        buffer_->readableBytes_ += count;
        buffer_->firstWithData_->setReadPosition(buffer_->firstWithData_->readPosition() - count);
    }

    virtual bool Skip(int count) {
        return true;
    }

    virtual int64_t ByteCount() const {
        return originalSize_ - buffer_->readableBytes();
    }

private:
    Buffer *buffer_;

    size_t originalSize_;

    size_t bytes_;
};

NAMESPACE_END

#endif //LIBUVCPP_BUFFERSTREAM_H
