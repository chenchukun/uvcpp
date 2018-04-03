//
// Created by chenchukun on 18/4/3.
//

#ifndef LIBUVCPP_TIMEVAL_H
#define LIBUVCPP_TIMEVAL_H

#include "utility.h"
#include <assert.h>

NAMESPACE_START

class Timeval
{
public:
    friend Timeval operator+(const Timeval &lval, const Timeval &rval);

    friend Timeval operator-(const Timeval &lval, const Timeval &rval);

    friend bool operator>(const Timeval &lval, const Timeval &rval);

    friend bool operator>=(const Timeval &lval, const Timeval &rval);

    friend bool operator<(const Timeval &lval, const Timeval &rval);

    friend bool operator<=(const Timeval &lval, const Timeval &rval);

    friend bool operator==(const Timeval &lval, const Timeval &rval);

    friend bool operator!=(const Timeval &lval, const Timeval &rval);


    Timeval() {
        struct timeval now;
        gettimeofday(&now, NULL);
        microsecond_ = now.tv_sec*1000000 + now.tv_usec;
    }

    Timeval(uint64_t second, uint64_t microsecond)
        : microsecond_(microsecond + second * 1000000)
    {

    }

    Timeval(uint64_t microsecond)
            : microsecond_(microsecond)
    {

    }

    Timeval(const struct timeval &t) {
        microsecond_ = t.tv_sec*1000000 + t.tv_usec;
    }

    Timeval(const Timeval &t) {
        microsecond_ = t.microsecond_;
    }

    Timeval& operator=(const Timeval &t) {
        microsecond_ = t.microsecond_;
        return *this;
    }

    uint64_t second() const {
        return microsecond_ / 1000000;
    }

    uint64_t millisecond() const {
        return microsecond_ / 1000;
    }

    uint64_t microsecond() const {
        return microsecond_;
    }

    Timeval& operator+=(const Timeval &val);

    Timeval& operator-=(const Timeval &val);

private:
    uint64_t microsecond_;
};

inline Timeval operator+(const Timeval &lval, const Timeval &rval)
{
    return Timeval(lval.microsecond_ + rval.microsecond_);
}

inline Timeval operator-(const Timeval &lval, const Timeval &rval)
{
    assert(lval >= rval);
    return Timeval(lval.microsecond_ - rval.microsecond_);
}

inline bool operator>(const Timeval &lval, const Timeval &rval)
{
    return lval.microsecond_ > rval.microsecond_;
}

inline bool operator>=(const Timeval &lval, const Timeval &rval)
{
    return (lval > rval) || (lval == rval);
}

inline bool operator<(const Timeval &lval, const Timeval &rval)
{
    return lval.microsecond_ < rval.microsecond_;
}


inline bool operator<=(const Timeval &lval, const Timeval &rval)
{
    return (lval < rval) || (lval == rval);
}

inline bool operator==(const Timeval &lval, const Timeval &rval)
{
    return lval.microsecond_ == rval.microsecond_;
}

inline bool operator!=(const Timeval &lval, const Timeval &rval)
{
    return !(lval == rval);
}

inline Timeval& Timeval::operator+=(const Timeval &val)
{
    microsecond_ += val.microsecond_;
    return *this;
}

inline Timeval& Timeval::operator-=(const Timeval &val)
{
    assert(microsecond_ >= val.microsecond_);
    microsecond_ -= val.microsecond_;
    return *this;
}

NAMESPACE_END

#endif //LIBUVCPP_TIMEVAL_H
