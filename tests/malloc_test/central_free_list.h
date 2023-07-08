#pragma once


#include "st_common.h"
#include "span.h"

class CentralFreeList
{
public:
	CentralFreeList();
	~CentralFreeList();

public:
	int Init(SizeClass cl);
	int InsertRange(void* start, void* end, int num);
	int RemoveRange(void** start, void** end, int num);

	// ���ش�span��ȡ���˶��ٶ���
	int FetchFromOneSpan(int num, void** start, void** end);
	// �ȳ��Դ�span��ȡ��span�в��㣬��page heap����span,Ȼ���ٻ�ȡ
	int FetchFromOneSpanSafe(int num, void** start, void** end);

	int ReleaseListToSpans(void* start);
	int ReleaseToSpans(void* object);

	// ��page heap �з���span,�з�Ϊcache����
	int Populate();

	// ��ͼ����tc entry cache������
	bool TryMakeCacheSpace();
	// ����cache��С
	int ShrinkCache(bool is_force);

	// ͳ��������Ƭ��С���������ܺ�
	unsigned long long OverheadBytes() const;
	int TCTotalLength() const;

	unsigned long long free_obj_num() const { return free_obj_num_; }

	static bool EvictRandomSizeClass(SizeClass lock_cl, bool is_force);

public:
	struct TCEntry
	{
		void* head;
		void* tail;
	};

	// ������������������
	static const int kMaxNumTransferEntries = 64;

private:
	SizeClass cl_;
	Span empty_; // �Ѿ�ȫ�������ȥ��span
	Span noempty_; // ���ַ����ȥ��span, �����Լ�������
	unsigned long long num_spans_; // empty + noempty��span�ܺ�
	TCEntry tc_slots_[kMaxNumTransferEntries];
	int used_slots_; // ʹ�õ�tc_slots��Ŀ
	int cache_size_; // ��ǰ������slot��С
	int max_cache_size_; // ���������slot��С
	unsigned long long free_obj_num_; // ���ж�������,����span�ڵĶ�������
};
