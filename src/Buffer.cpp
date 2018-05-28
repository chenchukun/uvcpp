#include "Buffer.h"
#include <iostream>

using namespace std;

NAMESPACE_START

const size_t MIN_BUFFER_SIZE = 1024;

DataBlock::DataBlock(size_t len)
{
    size_t realSize = MIN_BUFFER_SIZE;
    while (realSize < len) {
        realSize *= 2;
    }
    buffer_.resize(realSize);
    read_ = write_ = 0;
}

void DataBlock::realign() {
    size_t rindex = readPosition();
    if (rindex > 0) {
        size_t index = 0;
        for (; rindex < writePosition(); ++rindex) {
            buffer_[index++] = buffer_[rindex];
        }
        setReadPosition(0);
        setWritePosition(index);
    }
}

Buffer::Buffer()
    : firstWithData_(list_.begin()),
      lastWithData_(list_.end()),
      readableBytes_(0),
      capacity_(0)
{
}

Buffer::Buffer(const Buffer &buffer)
    : list_(buffer.list_),
      firstWithData_(list_.begin()),
      lastWithData_(list_.end()),
      readableBytes_(buffer.readableBytes_),
      capacity_(buffer.capacity_)
{
    auto it = buffer.firstWithData_;
    auto mit = list_.begin();
    while (it != buffer.list_.end()) {
        if (it == buffer.firstWithData_) {
            firstWithData_ = mit;
        }
        if (it == buffer.lastWithData_) {
            lastWithData_ = mit;
        }
        ++mit;
        ++it;
    }
}

Buffer::Buffer(Buffer &&buffer)
    : firstWithData_(list_.begin()),
      lastWithData_(list_.end()),
      readableBytes_(0),
      capacity_(0)
{
    list_.swap(buffer.list_);
    std::swap(firstWithData_, buffer.firstWithData_);
    std::swap(lastWithData_, buffer.lastWithData_);
    // 不加这两行会出错?
    buffer.firstWithData_ = buffer.list_.begin();
    buffer.lastWithData_ = buffer.list_.end();
    std::swap(readableBytes_, buffer.readableBytes_);
    std::swap(capacity_, buffer.capacity_);
}

Buffer& Buffer::operator=(const Buffer &buffer)
{
    Buffer buf = buffer;
    swap(buf);
    return *this;
}

void Buffer::swap(Buffer &buffer)
{
    list_.swap(buffer.list_);
    std::swap(firstWithData_, buffer.firstWithData_);
    std::swap(lastWithData_, buffer.lastWithData_);
    std::swap(readableBytes_, buffer.readableBytes_);
    std::swap(capacity_, buffer.capacity_);
}

void Buffer::append(const void *ptr, size_t len)
{
    if (lastWithData_ == list_.end()) {
        list_.emplace_back(len);
        lastWithData_ = list_.end();
        --lastWithData_;
        capacity_ += lastWithData_->capacity();
        if (list_.size() == 1) {
            firstWithData_ = lastWithData_;
        }
    }

    const char *buf = static_cast<const char*>(ptr);
    if (lastWithData_->writeableBytes() >= len) {
        memcpy(lastWithData_->peek() + lastWithData_->writePosition(), buf, len);
        lastWithData_->setWritePosition(lastWithData_->writePosition() + len);
        readableBytes_ += len;
    }
    else if (lastWithData_->totalSpace() >= len) {
        lastWithData_->realign();
        memcpy(lastWithData_->peek() + lastWithData_->writePosition(), buf, len);
        lastWithData_->setWritePosition(lastWithData_->writePosition() + len);
        readableBytes_ += len;
    }
    else {
        size_t nwrite = lastWithData_->writeableBytes();
        if (nwrite > 0) {
            memcpy(lastWithData_->peek() + lastWithData_->writePosition(), buf, nwrite);
            lastWithData_->setWritePosition(lastWithData_->capacity());
            readableBytes_ += nwrite;
        }
        ++lastWithData_;
        append(buf + nwrite, len - nwrite);
    }
}

void Buffer::prepend(const void *ptr, size_t len)
{
    const char *buf = static_cast<const char*>(ptr);
    if (firstWithData_ == list_.end()) {
        list_.emplace_front(len);
        firstWithData_ = lastWithData_ = list_.begin();
        capacity_ += firstWithData_->capacity();
        size_t offset = firstWithData_->capacity() - len;
        memcpy(firstWithData_->peek() + offset, buf, len);
        firstWithData_->setWritePosition(firstWithData_->capacity());
        firstWithData_->setReadPosition(offset);
        readableBytes_ += len;
    }
    else if (firstWithData_->prependableBytes() >= len) {
        memcpy(firstWithData_->peek() + firstWithData_->prependableBytes() - len, buf, len);
        firstWithData_->setReadPosition(firstWithData_->prependableBytes() - len);
        readableBytes_ += len;
    }
    else if (firstWithData_->prependableBytes() > 0) {
        size_t remain = firstWithData_->prependableBytes();
        memcpy(firstWithData_->peek(), buf + len - remain, remain);
        len -= remain;
        readableBytes_ += remain;
        firstWithData_->setReadPosition(0);
        prepend(buf, len);
    }
    else if (firstWithData_->prependableBytes() == 0) {
        list_.emplace_front(len);
        firstWithData_ = list_.begin();
        capacity_ += firstWithData_->capacity();
        size_t offset = firstWithData_->capacity() - len;
        memcpy(firstWithData_->peek() + offset, buf, len);
        firstWithData_->setWritePosition(firstWithData_->capacity());
        firstWithData_->setReadPosition(offset);
        readableBytes_ += len;
    }
}

string Buffer::toString(size_t len) const
{
    assert(readableBytes() >= len);

    string str;
    str.reserve(len);
    auto it = firstWithData_;
    while (len > 0) {
        if (it->readableBytes() >= len) {
            str.insert(str.size(), it->peek() + it->readPosition(), len);
            len = 0;
        }
        else if (it->readableBytes() > 0) {
            str.insert(str.size(), it->peek() + it->readPosition(), it->readableBytes());
            len -= it->readableBytes();
        }
        ++it;
    }
    return str;
}

string Buffer::toString() const
{
    return toString(readableBytes());
}

void Buffer::discard(size_t len)
{
    size_t backlen = len;
    assert(len <= readableBytes());
    auto it = firstWithData_;
    while (len > 0) {
        if (it->readableBytes() >= len) {
            it->setReadPosition(it->readPosition() + len);
            len = 0;
        }
        else if (it->readableBytes() > 0) {
            len -= it->readableBytes();
            it->setReadPosition(it->readPosition() + it->readableBytes());
        }
        ++it;
    }
    it = list_.begin();
    while (it != lastWithData_) {
        if (it->readableBytes() > 0) {
            break;
        }
        auto tmp = it;
        ++it;
        list_.erase(tmp);
    }
    firstWithData_ = it;
    readableBytes_ -= backlen;
}

void Buffer::discardAll()
{
    list_ = list<DataBlock>();
    readableBytes_ = capacity_ = 0;
    firstWithData_ = list_.begin();
    lastWithData_ = list_.end();
}

string Buffer::retrieveAsString(size_t len)
{
    assert(readableBytes() >= len);
    string str = move(toString(len));
    discard(len);
    return str;
}

string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

void Buffer::all(vector<pair<char*, size_t> > &bufs) const
{
    auto it = firstWithData_;
    while (it != lastWithData_) {
        if (it->readableBytes() > 0) {
            auto item = make_pair(const_cast<char*>(it->peek() + it->readPosition()), it->readableBytes());
            bufs.push_back(item);
        }
        ++it;
    }
    if (it != list_.end() && it->readableBytes() > 0) {
        auto item = make_pair(const_cast<char*>(it->peek() + it->readPosition()), it->readableBytes());
        bufs.push_back(item);
    }
}

void Buffer::next(char* &data, size_t &len)
{
    list<DataBlock>::iterator it = lastWithData_;
    if (lastWithData_ == list_.end()) {
        list_.emplace_back();
        lastWithData_ = list_.end();
        --lastWithData_;
        capacity_ += lastWithData_->capacity();
        if (list_.size() == 1) {
            firstWithData_ = lastWithData_;
        }
        it = lastWithData_;
    }
    else if (lastWithData_->writeableBytes() == 0) {
        it = lastWithData_;
        ++it;
        if (it == list_.end()) {
            list_.emplace_back();
            --it;
            capacity_ += it->capacity();
        }
    }
    else if (lastWithData_->prependableBytes() >= lastWithData_->capacity() / 2) {
        lastWithData_->realign();
    }
    data = it->peek() + it->readPosition();
    len = it->writeableBytes();
}

void Buffer::extend(size_t len)
{
    auto it = lastWithData_;
    if (lastWithData_->writeableBytes() == 0) {
        ++it;
    }
    it->setWritePosition(it->writePosition() + len);
    readableBytes_ += len;
}

void Buffer::unwrite(size_t len)
{
    assert(len <= lastWithData_->readableBytes());
    lastWithData_->setWritePosition(lastWithData_->writePosition() - len);
    readableBytes_ -= len;
}

void unread(size_t len)
{

}

void Buffer::debug() const
{
    auto it = list_.begin();
    cout << "----------------------------------" << endl;
    cout << "firstWithData = " << &(*firstWithData_) << endl;
    cout << "lastWithData = " << &(*lastWithData_) << endl;
    while (it != list_.end()) {
        cout << &(*it) << ": readPosition = " << it->readPosition() << ", writePosition = " << it->writePosition() << endl;
        ++it;
    }
    cout << "----------------------------------" << endl;
}

NAMESPACE_END