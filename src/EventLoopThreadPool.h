#ifndef LIBUVCPP_EVENTLOOPTHREADPOOL_H
#define LIBUVCPP_EVENTLOOPTHREADPOOL_H

#include "utility.h"
#include "EventLoop.h"
#include <vector>
#include <assert.h>
#include <thread>
#include <mutex>
#include <condition_variable>

NAMESPACE_START

class EventLoopThreadPool
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(int threadNum)
        : threadNum_(threadNum)
    {
        assert(threadNum_ > 0);
    }

    ~EventLoopThreadPool();

    void start(ThreadInitCallback initCallback=ThreadInitCallback());

    std::vector<EventLoop*> getAllLoops() {
        return loops_;
    }

    EventLoop* getLoop(size_t index) {
        assert(loops_.size() > 0);
        return loops_[index % threadNum_];
    }

private:
    std::vector<EventLoop*> loops_;

    std::vector<std::thread*> threads_;

    std::mutex mutex_;

    std::condition_variable cond_;

    int threadNum_;
};

NAMESPACE_END

#endif //LIBUVCPP_EVENTLOOPTHREADPOOL_H
