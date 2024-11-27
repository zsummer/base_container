#pragma once
#include "fn_log.h"






typedef unsigned long long PageID;
typedef unsigned long long Length;
typedef unsigned int SizeClass;

#define  error_tlog LOGFMTE
#define  debug_tlog LOGFMTD
#define  infor_tlog LOGFMTI

#ifdef WIN32
/*
* left to right scan
* num:<1>  return 0
* num:<2>  return 1
* num:<3>  return 1
* num:<4>  return 2
* num:<0>  return (unsigned int)-1
*/

inline unsigned int test_first_bit_index(unsigned long long num)
{
    DWORD index = (DWORD)-1;
    _BitScanReverse64(&index, num);
    return (unsigned int)index;
}
/*
* right to left scan
*/

inline unsigned int test_last_bit_index(unsigned long long num)
{
    DWORD index = -1;
    _BitScanForward64(&index, num);
    return (unsigned int)index;
}

#else
#define test_first_bit_index(num) ((unsigned int)(sizeof(unsigned long long) * 8 - __builtin_clzll((unsigned long long)num) - 1))
#define test_last_bit_index(num) ((unsigned int)(__builtin_ctzll((unsigned long long)num)))
#endif


// ����һ������2Ϊ�׵Ķ���, ���value����2^N�η�����ô������ȡ��
inline int log2_ceil(unsigned long long value)
{
    int n = test_first_bit_index(value);
    unsigned long long mask = (1ull << n) - 1;
    unsigned long long remainder = (mask & value) + mask;
    return n + ((remainder >> n) & 1);
}



static const  size_t kPageShift = 13;
// ÿҳ�ĳߴ�
static const size_t kPageSize = 1 << kPageShift;


static const  size_t kNumClasses = 65;
// ������Ը�����Ҫ�޸ģ����ڴ���ٶ�֮��������
static const size_t kMinAlign = 16;

// thread cacheӵ�е�����ڴ��С
static const size_t kMaxThreadCacheSize = 4 << 20;

static const size_t kMaxSize = 256 * 1024;
static const size_t kAlignment = 8;
static const size_t kLargeSizeClass = 0;

// 1M���µ�ҳ�����־�ȷ���б�,��ʱ���ã����ǲ���page�Ļ������
static const size_t kMaxPages = 1 << (20 - kPageShift);
// ���е�thread cache�ڴ�����
static const size_t kDefaultOverallThreadCacheSize = 8u * kMaxThreadCacheSize;
// ÿ��thread cache��С�ߴ�
static const  size_t kMinThreadCacheSize = kMaxSize * 2;

// �������̼���length ����max_length, ������������󣬾ͻ�����free list
static const unsigned int kMaxOverages = 3;

// ÿ��FreeList�������󳤶�
static const int kMaxDynamicFreeListLength = 8192;

static const Length kMaxValidPages = (~static_cast<Length>(0)) >> kPageShift;

// 64λϵͳ����ʵ��ʹ�õ�ֻ��48λ�����Բ���Ҫ��ô��λ����PageMap
static const int kAddressBits = (sizeof(void*) < 8 ? (8 * sizeof(void*)) : 48);
// һ���߳̿��Դ�����һ���̻߳�ȡ���ڴ�����
static const size_t kStealAmount = 1 << 16;

// ���ֻ����ȡ��2��48�η��ĵ�ַ���������ĵ�ַ�϶�����Ч��
static const unsigned long long kMaxValidAddr = 1ull << 48;

// ��ȡָ����byte�����������page������ת��
inline Length GetPagesNum(size_t bytes)
{
    return ((bytes >> kPageShift) + (bytes & (kPageSize - 1)) > 0 ? 1 : 0);
}

// ÿ����thread cache ��center cache֮�䴫�ݵĶ�������
static const int kDefaultTransferNumObjects = 1024;


// ������������meta data��chunk, ������ҳ������������
// ����Ϊ�˺�buddy system��ϣ����ٲ��ܷ�������buddy system��СԪ�ش�С
void* MetaDataChunkAlloc(size_t bytes);

typedef void* (*AllocPagesPtr)(int order);
typedef int (*FreePagesPtr)(void* ptr, size_t size);

// ���÷���ҳ�������ҳ��ĺ���ָ�롣
int STSetPageOpPtrs(AllocPagesPtr alloc_ptr, FreePagesPtr free_ptr);

void* STAllocPages(int order);
int STFreePages(void* ptr, size_t size);

