#ifndef LIBUVCPP_BLOCKING_QUEUE_H
#define LIBUVCPP_BLOCKING_QUEUE_H

#include "utility.h"
#include <condition_variable>
#include <mutex>
#include <queue>

NAMESPACE_START

template<typename T>
class BlockingQueue
{
public:
	BlockingQueue() {}

    BlockingQueue(const BlockingQueue &) = delete;

    BlockingQueue& operator=(const BlockingQueue &) = delete;

	void put(const T &elem);

	T take();

	bool poll(T &elem, double second);

	uint32_t size() {
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size();
	}

	bool empty() {
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.empty();
	}
	
private:
	std::queue<T> queue_;

	std::mutex mutex_;

	std::condition_variable cond_;
};

template<typename T>
void BlockingQueue<T>::put(const T &elem)
{
	{
		std::lock_guard<std::mutex> locker(mutex_);
		queue_.push(elem);
	}
	cond_.notify_one();
}

template<typename T>
T BlockingQueue<T>::take()
{
	std::unique_lock<std::mutex> locker(mutex_);
	while (queue_.empty()) {
		cond_.wait(locker);
	}
	T elem = queue_.front();
	queue_.pop();
	return elem;
}

template<typename T>
bool BlockingQueue<T>::poll(T &elem, double second)
{
	std::unique_lock<std::mutex> locker(mutex_);
	while (queue_.empty()) {
		int ret = cond_.wait(locker, second);
		if (ret == ETIMEDOUT) {
			return false;
		}
	}
	elem = queue_.front();
	queue_.pop();
	return true;
}

NAMESPACE_END
#endif
