#pragma once

#include "free_list.h"
#include "st_common.h"

class FreeList;

class ThreadCache
{
public:
    ThreadCache();
    ~ThreadCache();

public:
    int Init();
    void CleanUp();

    int freelist_length(int cl) const
    {
        return list_[cl].length();
    }
    size_t size() { return size_; }
    void* Allocate(SizeClass cl, size_t size);
    int Deallocate(void* ptr, SizeClass cl);

private:
    // ��Centralȡһ�������б�Ȼ�󷵻�һ����������
    void* FetchFromCentralCache(SizeClass cl, size_t size);
    // �ѻ��淵��Central Cache
    int ReleaseToCentralCache(FreeList* src, SizeClass cl, int num);

    // ���������ڴ�
    int Scavenge();
    // ���������������
    int ReduceTooLongList(FreeList* list, size_t cl);

    // ����thread cache���������޴�С
    int IncreaseCacheLimit();

private:
    size_t size_;     // ����λ�����ȥ�������ܴ�С
    size_t max_size_; // ���������С
    FreeList list_[kNumClasses];
};
