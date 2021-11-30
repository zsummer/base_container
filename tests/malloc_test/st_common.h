#pragma once
#include "fn_log.h"
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


typedef u64 PageID;
typedef u64 Length;
typedef u32 SizeClass;

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
* num:<0>  return (u32)-1
*/

inline u32 test_first_bit_index(u64 num)
{
    DWORD index = (DWORD)-1;
    _BitScanReverse64(&index, num);
    return (u32)index;
}
/*
* right to left scan
*/

inline u32 test_last_bit_index(u64 num)
{
    DWORD index = -1;
    _BitScanForward64(&index, num);
    return (u32)index;
}

#else
#define test_first_bit_index(num) ((u32)(sizeof(u64) * 8 - __builtin_clzll((u64)num) - 1))
#define test_last_bit_index(num) ((u32)(__builtin_ctzll((u64)num)))
#endif


// ����һ������2Ϊ�׵Ķ���, ���value����2^N�η�����ô������ȡ��
inline s32 log2_ceil(u64 value)
{
    s32 n = test_first_bit_index(value);
    u64 mask = (1ull << n) - 1;
    u64 remainder = (mask & value) + mask;
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
static const u32 kMaxOverages = 3;

// ÿ��FreeList�������󳤶�
static const int kMaxDynamicFreeListLength = 8192;

static const Length kMaxValidPages = (~static_cast<Length>(0)) >> kPageShift;

// 64λϵͳ����ʵ��ʹ�õ�ֻ��48λ�����Բ���Ҫ��ô��λ����PageMap
static const int kAddressBits = (sizeof(void*) < 8 ? (8 * sizeof(void*)) : 48);
// һ���߳̿��Դ�����һ���̻߳�ȡ���ڴ�����
static const size_t kStealAmount = 1 << 16;

// ���ֻ����ȡ��2��48�η��ĵ�ַ���������ĵ�ַ�϶�����Ч��
static const u64 kMaxValidAddr = 1ull << 48;

// ��ȡָ����byte�����������page������ת��
inline Length GetPagesNum(size_t bytes)
{
	return ((bytes >> kPageShift) + (bytes & (kPageSize - 1)) > 0 ? 1 : 0);
}

// ÿ����thread cache ��center cache֮�䴫�ݵĶ�������
static const s32 kDefaultTransferNumObjects = 1024;


// ������������meta data��chunk, ������ҳ������������
// ����Ϊ�˺�buddy system��ϣ����ٲ��ܷ�������buddy system��СԪ�ش�С
void* MetaDataChunkAlloc(size_t bytes);

typedef void* (*AllocPagesPtr)(s32 order);
typedef s32 (*FreePagesPtr)(void* ptr, size_t size);

// ���÷���ҳ�������ҳ��ĺ���ָ�롣
s32 STSetPageOpPtrs(AllocPagesPtr alloc_ptr, FreePagesPtr free_ptr);

void* STAllocPages(s32 order);
s32 STFreePages(void* ptr, size_t size);

