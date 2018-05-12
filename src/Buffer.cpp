#include "Buffer.h"

using namespace std;

NAMESPACE_START

Buffer::Buffer(const Buffer &buffer)
{
    auto it = buffer.blocks_.cbegin();
    while (it != buffer.blocks_.cend()) {
        blocks_.push_back(*it);
        ++it;
    }
}

Buffer::Buffer(Buffer &&buffer)
{
    blocks_.swap(buffer.blocks_);
}

size_t Buffer::readableBytes() const
{
    size_t size = 0;
    auto it = blocks_.cbegin();
    while (it != blocks_.cend()) {
        size += it->readableBytes();
        ++it;
    }
    return size;
}

size_t Buffer::writableBytes() const
{
    if (blocks_.empty()) {
        return 0;
    }
    size_t bytes = 0;
    auto it = blocks_.crbegin();
    while (it != blocks_.crend() && it->empty()) {
        bytes += it->writableBytes();
        ++it;
    }
    if (it != blocks_.crend()) {
        bytes += it->writableBytes();
    }
    return bytes;
}

size_t Buffer::prependSpaceBytes() const
{
    if (blocks_.empty()) {
        return 0;
    }
    size_t bytes = 0;
    auto it = blocks_.cbegin();
    while (it != blocks_.cend() && it->empty()) {
        bytes += it->writableBytes();
        ++it;
    }
    if (it != blocks_.cend()) {
        bytes += it->prependSpaceBytes();
    }
    return bytes;
}

void Buffer::discard(size_t size)
{
    assert(size <= readableBytes());
    auto it = blocks_.begin();
    while (it != blocks_.end() && size > 0) {
        size_t bytes = it->readableBytes();
        if (bytes <= size) {
            blocks_.pop_front();
            it = blocks_.begin();
            size -= bytes;
            continue;
        }
        it->setReadPosition(it->readPosition() + size);
        break;
    }
}


string Buffer::retrieveAsString(size_t size)
{
    assert(size <= readableBytes());
    string str;
    str.reserve(size);
    auto it = blocks_.begin();
    while (it != blocks_.end() && size > 0) {
        const char *peek = it->peek();
        size_t bytes = it->readableBytes();
        size_t readPos = it->readPosition();
        if (bytes >= size) {
            str.insert(str.end(), peek, peek + size);
            it->setReadPosition(readPos + size);
            size = 0;
            if (it->empty()) {
                blocks_.pop_front();
            }
        }
        else {
            str.insert(str.end(), peek, peek + bytes);
            blocks_.pop_front();
            it = blocks_.begin();
            size -= bytes;
        }
    }
    return move(str);
}

string Buffer::peekAsString(size_t size) const
{
    assert(size <= readableBytes());
    string str;
    str.reserve(size);
    auto it = blocks_.begin();
    while (it != blocks_.end()  && size > 0) {
        const char *peek = it->peek();
        size_t bytes = it->readableBytes();
        if (bytes > size) {
            str.insert(str.end(), peek, peek + size);
            size = 0;
        }
        else {
            str.insert(str.end(), peek, peek + bytes);
            size -= bytes;
        }
        ++it;
    }
    return move(str);
}

void Buffer::append(const void *ptr, size_t len)
{
    ensureWritableBytes(len);
    auto point = appendPoint();
    auto it = point.first;
    const char *buf = static_cast<const char*>(ptr);
    while (len > 0 && it != blocks_.end()) {
        size_t bytes = it->writableBytes();
        if (bytes >= len) {
            memcpy(it->peek() + it->readableBytes(), buf, len);
            it->setWritePosition(it->writePosition() + len);
            len = 0;
        }
        else {
            memcpy(it->peek() + it->readableBytes(), buf, bytes);
            it->setWritePosition(it->writePosition() + bytes);
            len -= bytes;
            buf += bytes;
            ++it;
        }
    }
}

void Buffer::prepend(const void *ptr, size_t len)
{
    ensurePrependBytes(len);
    auto point = prependPoint(len);
    auto it = point.first;
    size_t offset = point.second;
    const char *buf = static_cast<const char*>(ptr);
    while (len > 0) {
        if (it->empty()) {
            size_t bytes = it->writableBytes() < len? it->writableBytes(): len;
            memcpy(it->head() + offset, buf, bytes);
            it->setWritePosition(bytes);
            len -= bytes;
            buf += bytes;
            offset = 0;
        }
        else {
            memcpy(it->head() + offset, buf, len);
            it->setReadPosition(it->readPosition() - len);
            len = 0;
        }
        ++it;
    }
}

pair<deque<Block>::iterator, size_t> Buffer::appendPoint()
{
    if (readableBytes() == 0) {
        return pair<deque<Block>::iterator, size_t>(blocks_.begin(), 0);
    }
    auto it = blocks_.begin();
    size_t bytes = readableBytes();
    size_t count = 0;
    while (it != blocks_.end()) {
        count += it->readableBytes();
        if (count >= bytes) {
            break;
        }
    }
    assert(it != blocks_.end());
    if (it->writableBytes() == 0) {
        return pair<deque<Block>::iterator, size_t>(++it, 0);
    }
    else {
        return pair<deque<Block>::iterator, size_t>(it, it->writePosition());
    }
}

pair<deque<Block>::iterator, size_t> Buffer::prependPoint(size_t size)
{
    if (readableBytes() == 0) {
        return pair<deque<Block>::iterator, size_t>(blocks_.begin(), 0);
    }
    auto it = blocks_.begin();
    while (it != blocks_.end() && it->empty()) {
        ++it;
    }
    assert(it != blocks_.end());
    if (it->prependSpaceBytes() >= size) {
        return pair<deque<Block>::iterator, size_t>(it, it->readPosition() - size);
    }
    size_t count = size - it->prependSpaceBytes();

    while (it != blocks_.begin()) {
        --it;
        if (it->writableBytes() >= count) {
            return pair<deque<Block>::iterator, size_t>(it, it->writableBytes() - count);
        }
        else {
            count -= it->writableBytes();
        }
    }
    assert(false);
}

void Buffer::initUVBuffer(std::vector<uv_buf_t> &bufs) const {
    auto it = blocks_.begin();
    while (it != blocks_.end()) {
        if (!it->empty()) {
            bufs.push_back(uv_buf_t());
            bufs.back().base = const_cast<char*>(it->peek());
            bufs.back().len = it->readableBytes();
        }
        ++it;
    }
}

void Buffer::initUVBuffer(uv_buf_t *buf)
{
    if (writableBytes() == 0) {
        blocks_.emplace_back(initSize);
        buf->base = blocks_.back().head();
        buf->len = initSize;
    }
    else {
        auto point = appendPoint();
        auto it = point.first;
        size_t bytes = it->writableBytes();
        buf->base = it->head() + it->writePosition();
        buf->len = bytes;
    }

}

NAMESPACE_END