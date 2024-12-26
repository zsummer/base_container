
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the zbase, used MIT License.
*/




#pragma once 
#ifndef ZCLOCK_DIAGNOSTIC_H
#define ZCLOCK_DIAGNOSTIC_H

#include "zclock.h"


template<zclock_impl::clock_type _C = zclock_impl::T_CLOCK_VOLATILE_RDTSC>
class zclock_diagnostic_ns
{
public:
    //watchdog  ns  
    explicit zclock_diagnostic_ns(long long watchdog, std::function<void(long long)> dog)
    {
        watchdog_ = watchdog;
        dog_ = dog;
        clock_.start();
    }
    ~zclock_diagnostic_ns()
    {
        if (dog_ == nullptr)
        {
            return;
        }
        long long ns = clock_.save().cost_ns();
        if (ns >= watchdog_)
        {
            dog_(ns);
        }
    }
private:
    zclock<_C> clock_;
    long long watchdog_;
    std::function<void(long long)> dog_;
};

template<zclock_impl::clock_type _C = zclock_impl::T_CLOCK_VOLATILE_RDTSC>
class zclock_diagnostic_ms 
{
public:
public:
    //watchdog  ms  
    explicit zclock_diagnostic_ms(long long watchdog, std::function<void(long long)> dog)
    {
        watchdog_ = watchdog * 1000 * 1000;
        dog_ = dog;
        clock_.start();
    }
    ~zclock_diagnostic_ms()
    {
        if (dog_ == nullptr)
        {
            return;
        }
        long long ns = clock_.save().cost_ns();
        if (ns >= watchdog_)
        {
            dog_(ns/1000/1000);
        }
    }
private:
    zclock<_C> clock_;
    long long watchdog_;
    std::function<void(long long)> dog_;
};





#endif