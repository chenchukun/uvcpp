#ifndef LIBUVCPP_EVENTLOOP_H
#define LIBUVCPP_EVENTLOOP_H

#include <cstdlib>
#include <uv.h>
#include <functional>
#include <thread>
#include <mutex>
#include "utility.h"
#include "Any.h"
#include "Timeval.h"

NAMESPACE_START

typedef std::function<void(void)> AsyncCallback;

typedef std::function<void(void)> TimerCallback;

class EventLoop
{
public:

    EventLoop(const EventLoop&) = delete;

    EventLoop& operator=(const EventLoop&) = delete;

    EventLoop();

    ~EventLoop();

    // 执行事件循环
    void run(uv_run_mode mode = UV_RUN_DEFAULT) {
        while (run_) {
            uv_run(loop_, mode);
        }
    }

    // 退出事件循环
    void stop();

    // 获取libuv的uv_loop_t
    uv_loop_t* getLoop() {
        return loop_;
    }

    // 在事件循环所在线程执行回调
    void runInLoopThread(AsyncCallback cb);

    // 获取当前线程的EventLoop
    static EventLoop* getCurrThreadEventLoop();

    std::thread::id getThreadId() const {
        return threadId_;
    }

    uv_timer_t* runAt(Timeval time, TimerCallback cb);

    uv_timer_t* runAfter(uint64_t delay, TimerCallback cb);

    uv_timer_t* runEvery(uint64_t interval, TimerCallback cb);

    void cancel(uv_timer_t *timer);

    void setData(Any data) {
        data_.swap(data);
    }

    Any& getData() {
        return data_;
    }

private:
    static void asyncCallback(uv_async_t *async);

    static void timerCallback(uv_timer_t *timer);

    static void closeCallback(uv_handle_t* handle);

    static void signalCallback(uv_signal_t *handle, int signum) {

    }

    uv_timer_t* timerRunImpl(Timeval time, uint64_t interval, TimerCallback cb);

private:
    uv_loop_t *loop_;

    std::mutex mutex_;

    std::thread::id threadId_;

    bool run_;

    Any data_;

    uv_signal_t *signal_;
};

NAMESPACE_END

#endif //EVENTLOOP_H
