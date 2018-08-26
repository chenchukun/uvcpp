#ifndef LIBUVCPP_THREADPOOL_H
#define LIBUVCPP_THREADPOOL_H

#include "utility.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <assert.h>
#include <atomic>

NAMESPACE_START

class ThreadPool
{
public:
    typedef std::function<void(void)> Task;

    ThreadPool(): initCallback_(NULL), maxSize_(100), running_(false), activeThreadNum_(0) {}

    ~ThreadPool() {
        if (running_) {
            stop();
        }
    }

    ThreadPool(const ThreadPool&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;

    void start(int threadNum=4);

    void run(const Task &task);

    void stop();

    void setThreadInitCallback(const Task &task) {
        initCallback_ = task;
    }

    void setMaxQueueSize(size_t maxSize) {
        maxSize_ = maxSize;
    }

    size_t threadNum() const {
        return threads_.size();
    }

    size_t activeThreadNum() const {

    }

private:
    void threadFunc();

private:
    size_t maxSize_;

    bool running_;

    std::atomic<size_t> activeThreadNum_;

    Task initCallback_;

    std::vector<std::thread*> threads_;

    std::queue<Task> queue_;

    std::mutex mutex_;

    std::condition_variable notEmptyCond_;

    std::condition_variable notFullCond_;

};

NAMESPACE_END

#endif //LIBUVCPP_THREADPOOL_H
