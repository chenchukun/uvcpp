#ifndef LIBUVCPP_SINGLETON_H
#define LIBUVCPP_SINGLETON_H

#include "utility.h"
#include <mutex>

NAMESPACE_START

template <typename T>
class Singleton
{
public:
    Singleton() = delete;

    Singleton& operator=(const Singleton&) = delete;

    Singleton(const Singleton&) = delete;

    ~Singleton() = delete;

    static T& instance() {
        std::call_once(flag_, &Singleton::init);
        return *instance_;
    }

private:
    static void init() {
        instance_ = new T;
    }

private:
    static T *instance_;

    static std::once_flag flag_;
};

template<typename T>
std::once_flag Singleton<T>::flag_;

template<typename T>
T* Singleton<T>::instance_ = NULL;

NAMESPACE_END

#endif //LIBUVCPP_SINGLETON_H
