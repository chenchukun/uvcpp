#ifndef LIBUVCPP_THREADLOCAL_H
#define LIBUVCPP_THREADLOCAL_H

#include "utility.h"
#include <uv.h>

NAMESPACE_START

template<typename T>
class ThreadLocal
{
public:
    ThreadLocal() {
        key_ = static_cast<uv_key_t *>(malloc(sizeof(uv_key_t)));
        uv_key_create(key_);
    }

    ~ThreadLocal() {
        T *p = static_cast<T*>(uv_key_get(key_));
        if (p != NULL) {
            delete p;
        }
        uv_key_delete(key_);
    }

    ThreadLocal(const ThreadLocal&) = delete;

    ThreadLocal& operator=(const ThreadLocal&) = delete;

    T& value() {
        T *p = static_cast<T*>(uv_key_get(key_));
        if (p == NULL) {
            T *value = new T;
            uv_key_set(key_, static_cast<void*>(value));
            p = value;
        }
        return *p;
    }

private:
    uv_key_t *key_;
};

NAMESPACE_END

#endif //LIBUVCPP_THREADLOCAL_H
