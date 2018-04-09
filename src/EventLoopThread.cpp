#include "EventLoopThread.h"
#include <iostream>
#include <assert.h>
using namespace std;

NAMESPACE_START

EventLoop* EventLoopThread::startLoop()
{
    assert(thread_ == NULL);
    thread_ = new thread([this]() {
        EventLoop *loop = new EventLoop;
        if (this->initCallback_) {
            this->initCallback_(loop);
        }
        this->loop_ = loop;
        this->cond_.notify_all();
        loop->run();

    });
    unique_lock<mutex> locker(mutex_);
    while (loop_ == NULL) {
        cond_.wait(locker);
    }
    return loop_;
}

EventLoopThread::~EventLoopThread()
{
    if (thread_) {
        loop_->stop();
        thread_->join();
        delete thread_;
        delete loop_;
    }
}

NAMESPACE_END