#include "ThreadPool.h"
using namespace std;

NAMESPACE_START

void ThreadPool::start(int threadNum) {
    assert(!running_ && threadNum > 0 && maxSize_>0);
    activeThreadNum_ = 0;
    threads_.reserve(threadNum);
    for (int i=0; i<threadNum; ++i) {
        threads_.push_back(new thread(bind(&ThreadPool::threadFunc, this)));
    }
    running_ = true;
}

void ThreadPool::run(const Task &task)
{
    unique_lock<mutex> locker(mutex_);
    while (queue_.size()>=maxSize_ && running_) {
        notFullCond_.wait(locker);
    }
    if (running_) {
        queue_.push(task);
        notEmptyCond_.notify_all();
    }
}

void ThreadPool::threadFunc()
{
    if (initCallback_) {
        initCallback_();
    }
    while (running_) {
        Task task = NULL;
        {
            unique_lock<mutex> locker(mutex_);
            if (queue_.empty()) {
                notEmptyCond_.wait(locker);
            }
            if (!queue_.empty()) {
                task = queue_.front();
                queue_.pop();
                notFullCond_.notify_all();
            }
        }
        if (task) {
            ++activeThreadNum_;
            task();
            --activeThreadNum_;
        }
    }
}

void ThreadPool::stop()
{
    unique_lock<mutex> locker(mutex_);
    running_ = false;
    notEmptyCond_.notify_all();
    for (int i=0; i<threads_.size(); ++i) {
        threads_[i]->join();
        delete threads_[i];
    }
    threads_.clear();
    while (!queue_.empty()) {
        queue_.pop();
    }
}

NAMESPACE_END