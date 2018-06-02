#ifndef LIBUVCPP_OBJECTPOOL_H
#define LIBUVCPP_OBJECTPOOL_H

#include "utility.h"
#include <mutex>

NAMESPACE_START

template<typename T>
class ObjectPool
{
public:
    ObjectPool(size_t poolSize)
        : poolSize_(poolSize)
    {

    }

    T* allocate() {
        T *obj = NULL;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!objPoolList_.empty()) {
                obj = objPoolList_.front();
                objPoolList_.pop_front();
            }
        }
        obj = new T();
        return obj;
    }

    void release(T *obj) {
        T *delObj = NULL;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (objPoolList_.size() >= poolSize_) {
                delObj = obj;
            }
            else {
                objPoolList_.push_back(obj);
            }
        }
        if (delObj) {
            delete delObj;
        }
    }

    ~ObjectPool() {
        while (!objPoolList_.empty()) {
            delete objPoolList_.front();
            objPoolList_.pop_front();
        }
    }

private:
    size_t poolSize_;

    std::list<T*> objPoolList_;

    std::mutex mutex_;
};

NAMESPACE_END

#endif //LIBUVCPP_OBJECTPOOL_H
