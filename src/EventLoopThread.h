#ifndef LIBUVCPP_EVENTLOOPTHREAD_H
#define LIBUVCPP_EVENTLOOPTHREAD_H

#include "EventLoop.h"
#include "utility.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

NAMESPACE_START

class EventLoopThread
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(ThreadInitCallback callback=ThreadInitCallback())
        : initCallback_(callback),
          loop_(NULL),
          thread_(NULL)
    {

    }

    ~EventLoopThread();

    EventLoopThread(const EventLoopThread&) = delete;

    EventLoopThread& operator=(const EventLoopThread&) = delete;

    EventLoop* startLoop();

private:
    EventLoop *loop_;

    std::mutex mutex_;

    std::condition_variable cond_;

    ThreadInitCallback initCallback_;

    std::thread *thread_;
};

NAMESPACE_END

#endif //LIBUVCPP_EVENTLOOPTHREAD_H
