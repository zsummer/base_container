

/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the zbase, used MIT License.
*/



#pragma once 
#ifndef ZTRACE_H
#define ZTRACE_H


#include <stdint.h>
#include <type_traits>
#include <iterator>
#include <cstddef>
#include <memory>
#include <algorithm>


//default use format compatible short type .  
#if !defined(ZBASE_USE_AHEAD_TYPE) && !defined(ZBASE_USE_DEFAULT_TYPE)
#define ZBASE_USE_DEFAULT_TYPE
#endif 

//win & unix format incompatible   
#ifdef ZBASE_USE_AHEAD_TYPE
using s8 = int8_t;
using u8 = uint8_t;
using s16 = int16_t;
using u16 = uint16_t;
using s32 = int32_t;
using u32 = uint32_t;
using s64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
#endif

#ifdef ZBASE_USE_DEFAULT_TYPE
using s8 = char;
using u8 = unsigned char;
using s16 = short int;
using u16 = unsigned short int;
using s32 = int;
using u32 = unsigned int;
using s64 = long long;
using u64 = unsigned long long;
using f32 = float;
using f64 = double;
#endif


#if __GNUG__
#define ZBASE_ALIAS __attribute__((__may_alias__))
#else
#define ZBASE_ALIAS
#endif

/* type_traits:
*
* is_trivially_copyable: in part
    * memset: uninit or no dync heap
    * memcpy: uninit or no dync heap
* shm resume : safely, require heap address fixed
    * has vptr:     no
    * static var:   no
    * has heap ptr: yes
    * has code ptr: no
* thread safe: read safe
*
*/




//��Ե�����׷�� ����ͬ�������г��ֹ���ĵ��û��ߵݹ�ѭ��, ���糣�����¼�ϵͳ, ����ϵͳ, BUFFϵͳ��. �������һ��ջ֡����ʹ��ҵ��ID.  
// * �����ֵ�����������ʱ, ͨ��ջ֡���Ƿ������ͬID�ж�Ϊ���ֵݹ�ѭ��, ����ͨ��Ϊ����������(�������ȡ����ҵ��)        
// * �����ֵ�����������ʱ, ͨ��Ӧ�������˳���������ֹ����(С���жϺʹ�����Դй¶����).   
// * �����в�Ӧ����ѯջ����, ֻͨ���Ƿ�������ж�, ��������¼����  
// 
//��Ե�����׷�� �ڳ��ִ���ʱ���ӡid���ߵ�ַ�����ֵ��ù�ϵ   
// * �����ֵ�����������ʱ, ���Բ鿴ջ���������ж����һ����ջ��¼(δ���ǵ�ջ֡��ʷ��¼)      
// * �����ֵ�����������ʱ, ���Բ鿴��ǰ��ջ��Դ 
// * ��Ҫ��ÿ����ö�guard, ȡ�����Ƿ���Ҫ��ע    
// * ����ջ������__FUNCTION__�ĵ�ַ��, ����ֱ�ӹ۲쵽�������ù�ϵ ����ΪPIC/PIE���ܴ洢��shm  
// * Ҳ���Դ洢ҵ����ԴID, �����Ų���õĹؼ���Դ  


template<class _Ty, s32 _Depth>
class ztrace_stacker
{
public:
    enum TRACE_ERRCODE
    {
        TE_NONE,
        TE_STACKOVER, //��Ҫ�ֶ�����  
        TE_MISMATCHING, //ʹ�÷�ʽ���� 
    };

    using value_type = _Ty;
    static constexpr s32 max_depth = _Depth;

public:
    ztrace_stacker()
    {
        static_assert(std::is_trivial<_Ty>::value, "");
        reset(false);
    }
    void reset(bool light_clear = true)
    {
        if (!light_clear)
        {
            memset(stacker_, 0, sizeof(stacker_));
        }
        size_ = 0;
        errcode_ = TE_NONE;
    }
    bool good() const 
    {
        return errcode_ == TE_NONE;
    }
    void set_errcode(s32 errcode = TE_NONE) { errcode_ = errcode; }
    s32 errcode()const
    {
        return errcode_;
    }
    void push(const _Ty& v)
    {
        if (errcode_ != TE_NONE)
        {
            return;
        }
        if (size_ >= _Depth)
        {
            errcode_ = TE_STACKOVER;
            return;
        }
        stacker_[size_++] = v;
    }
    void pop(const _Ty& v)
    {
        if (errcode_ != TE_NONE)
        {
            return;
        }
        if (size_ <= 0)
        {
            errcode_ = TE_MISMATCHING;
            return;
        }

        const _Ty& vtop = stacker_[size_ - 1];
        if (vtop != v)
        {
            errcode_ = TE_MISMATCHING;
            return;
        }
        size_--;
    }
    void set_top(s32 id) { size_ = id + 1; }
    //ָ��ջ������   
    s32 top()const { return size_ - 1; }
    s32 max_top()const { return max_depth -1; }
    const _Ty& at(s32 id)const { return stacker_[id]; }
    
private:
    _Ty stacker_[_Depth];
    s32 size_;
    s32 errcode_;
};



template<class _Stacker>
class ztrace_guard
{
public:
    ztrace_guard() = delete;
    ztrace_guard<_Stacker>& operator=(ztrace_guard<_Stacker>&) = delete;
    explicit ztrace_guard(_Stacker& stacker, const typename _Stacker::value_type& v) :stacker_(stacker), v_(v)
    {
        stacker_.push(v_);
    }

    ~ztrace_guard()
    {
        stacker_.pop(v_);
    }
private:
    _Stacker& stacker_;
    const typename _Stacker::value_type v_;
};



//don't store to shm 
template<s32 _Depth>
using zcallstacker = ztrace_stacker<const char*, _Depth>;







#endif