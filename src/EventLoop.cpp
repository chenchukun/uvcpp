#include "EventLoop.h"
#include <assert.h>
#ifndef _WIN32
#include <unistd.h>
#endif
using namespace std;

NAMESPACE_START

__thread EventLoop *currEventLoop = NULL;

EventLoop::EventLoop()
{
    threadId_ = std::this_thread::get_id();
    assert(currEventLoop == NULL);
    loop_ = static_cast<uv_loop_t*>(malloc(sizeof(uv_loop_t)));
    uv_loop_init(loop_);
    assert(currEventLoop == NULL);
    currEventLoop = this;
}

EventLoop::~EventLoop()
{
    uv_loop_close(loop_);
    free(loop_);
}

void EventLoop::stop()
{
    uv_loop_t *loop = loop_;
    runInLoopThread([loop] () {
        uv_stop(loop);
    });
}

void EventLoop::runInLoopThread(AsyncCallback cb)
{
    if (getThreadId() == this_thread::get_id()) {
        cb();
    }
    else {
        uv_async_t *async = static_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
        async->data = NULL;
        if (cb != NULL) {
            async->data = static_cast<void*>(new AsyncCallback(cb));
        }
        lock_guard<mutex> guard(mutex_);
        uv_async_init(loop_, async, EventLoop::asyncCallback);
        uv_async_send(async);
    }
}

uv_timer_t* EventLoop::timerRunImpl(struct timeval time, uint32_t interval, TimerCallback cb)
{
    if (cb != NULL) {
        TIMEVAL_NOW(now);

        TIMEVAL_DIFF(diff, time, now);

        uv_timer_t *timer = static_cast<uv_timer_t*>(malloc(sizeof(uv_timer_t)));
        timer->data = static_cast<void*>(new TimerCallback(cb));
        uv_loop_t *loop = loop_;
        runInLoopThread([loop, timer, diff, interval] () {
            uv_timer_init(loop, timer);
            uv_timer_start(timer, EventLoop::timerCallback, diff.tv_sec*1000 + diff.tv_usec/1000, interval);
        });
        return timer;
    }
    return NULL;
}

uv_timer_t* EventLoop::runAt(struct timeval time, TimerCallback cb)
{
    return timerRunImpl(time, 0, cb);
}

uv_timer_t* EventLoop::runAfter(uint32_t delay, TimerCallback cb)
{
    TIMEVAL_NOW(now);
    struct timeval tvdelay{delay/1000, (int32_t)((delay*1000)%1000000)};
    return timerRunImpl(tvdelay, 0, cb);
}

uv_timer_t* EventLoop::runEvery(uint32_t interval, TimerCallback cb)
{
    TIMEVAL_NOW(now);
    struct timeval tvint{interval/1000, (int32_t)((interval*1000)%1000000)};
    TIMEVAL_ADD(tv, now, tvint);
    return timerRunImpl(tv, interval, cb);
}

void EventLoop::cancel(uv_timer_t *timer)
{
    runInLoopThread([timer] () {
        uv_timer_stop(timer);
        free(timer->data);
        free(timer);
    });
}

EventLoop* EventLoop::getCurrThreadEventLoop()
{
    assert(currEventLoop != NULL);
    return currEventLoop;
}

void EventLoop::asyncCallback(uv_async_t *async)
{
    if (async->data != NULL) {
        AsyncCallback *cb = static_cast<AsyncCallback*>(async->data);
        (*cb)();
        delete cb;
    }
    uv_close((uv_handle_t*)async, EventLoop::closeCallback);
}

void EventLoop::timerCallback(uv_timer_t *timer)
{
    if (timer->data != NULL) {
        TimerCallback *cb = static_cast<TimerCallback*>(timer->data);
        (*cb)();
    }
    uint64_t repeat = uv_timer_get_repeat(timer);
    if (repeat == 0) {
        uv_timer_stop(timer);
        delete static_cast<TimerCallback*>(timer->data);
        free(timer);
    }
}

void EventLoop::closeCallback(uv_handle_t* handle)
{
    free(handle);
}

NAMESPACE_END
