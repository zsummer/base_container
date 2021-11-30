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
	s32 Init();
	void CleanUp();

	s32 freelist_length(s32 cl) const
	{
		return list_[cl].length();
	}
	size_t size() { return size_; }
	void* Allocate(SizeClass cl, size_t size);
	s32 Deallocate(void* ptr, SizeClass cl);

private:
	// ��Centralȡһ�������б�Ȼ�󷵻�һ����������
	void* FetchFromCentralCache(SizeClass cl, size_t size);
	// �ѻ��淵��Central Cache
	s32 ReleaseToCentralCache(FreeList* src, SizeClass cl, s32 num);

	// ���������ڴ�
	s32 Scavenge();
	// ���������������
	s32 ReduceTooLongList(FreeList* list, size_t cl);

	// ����thread cache���������޴�С
	s32 IncreaseCacheLimit();

private:
	size_t size_;     // ����λ�����ȥ�������ܴ�С
	size_t max_size_; // ���������С
	FreeList list_[kNumClasses];
};
