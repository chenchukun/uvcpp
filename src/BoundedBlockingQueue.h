#ifndef LIBUVCPP_BOUNDEDBLOCKINGQUEUE_H
#define LIBUVCPP_BOUNDEDBLOCKINGQUEUE_H

#include "utility.h"
#include <queue>
#include <mutex>
#include <condition_variable>

NAMESPACE_START

template <typename T>
class BoundedBlockingQueue
{
public:
    BoundedBlockingQueue(size_t maxSize): maxSize_(maxSize) {}

    BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;

    BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;

    void put(const T &elem);

    T take();

    size_t size() const {
        return queue_.size();
    }

    size_t capacity() const {
        return maxSize_;
    }

    bool empty() const {
        std::lock_guard<std::mutex> locker(mutex_);
        return queue_.empty();
    }

    bool full() const {
        std::lock_guard<std::mutex> locker(mutex_);
        return size() == capacity();
    }

private:
    std::queue<T> queue_;

    std::mutex mutex_;

    std::condition_variable notFullCond_;

    std::condition_variable notEmptyCond_;

    size_t maxSize_;

};

template<typename T>
void BoundedBlockingQueue<T>::put(const T &elem)
{
    std::unique_lock<std::mutex> locker(mutex_);
    while (queue_.size() == maxSize_) {
        notFullCond_.wait(locker);
    }
    queue_.push(elem);
    notEmptyCond_.notify_all();
}

template<typename T>
T BoundedBlockingQueue<T>::take()
{
    std::unique_lock<std::mutex> locker(mutex_);
    while (queue_.empty()) {
        notEmptyCond_.wait(locker);
    }
    T elem = queue_.front();
    queue_.pop();
    notFullCond_.notify_all();
    return elem;
}

NAMESPACE_END
#endif //LIBUVCPP_BOUNDEDBLOCKINGQUEUE_H
