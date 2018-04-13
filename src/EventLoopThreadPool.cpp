#include "EventLoopThreadPool.h"

using namespace std;

NAMESPACE_START

void EventLoopThreadPool::start(ThreadInitCallback initCallback)
{
    int initSuc = 0;
    for (int i=0; i<threadNum_; ++i) {
        thread *t = new thread([this, &initSuc, initCallback] {
            EventLoop *loop = new EventLoop();
            if (initCallback != NULL) {
                initCallback(loop);
            }
            this->mutex_.lock();
            this->loops_.push_back(loop);
            ++initSuc;
            this->mutex_.unlock();
            this->cond_.notify_all();
            loop->run();
        });
        threads_.push_back(t);
    }
    unique_lock<mutex> locker(mutex_);
    while (initSuc != threadNum_) {
        cond_.wait(locker);
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    for (size_t i = 0; i < loops_.size(); ++i) {
        loops_[i]->stop();
        threads_[i]->join();
        delete threads_[i];
        delete loops_[i];
    }
}

NAMESPACE_END