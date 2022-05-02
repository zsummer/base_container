#pragma once


#include "st_common.h"
#include "span.h"

class CentralFreeList
{
public:
	CentralFreeList();
	~CentralFreeList();

public:
	s32 Init(SizeClass cl);
	s32 InsertRange(void* start, void* end, s32 num);
	s32 RemoveRange(void** start, void** end, s32 num);

	// ���ش�span��ȡ���˶��ٶ���
	s32 FetchFromOneSpan(s32 num, void** start, void** end);
	// �ȳ��Դ�span��ȡ��span�в��㣬��page heap����span,Ȼ���ٻ�ȡ
	s32 FetchFromOneSpanSafe(s32 num, void** start, void** end);

	s32 ReleaseListToSpans(void* start);
	s32 ReleaseToSpans(void* object);

	// ��page heap �з���span,�з�Ϊcache����
	s32 Populate();

	// ��ͼ����tc entry cache������
	bool TryMakeCacheSpace();
	// ����cache��С
	s32 ShrinkCache(bool is_force);

	// ͳ��������Ƭ��С���������ܺ�
	u64 OverheadBytes() const;
	s32 TCTotalLength() const;

	u64 free_obj_num() const { return free_obj_num_; }

	static bool EvictRandomSizeClass(SizeClass lock_cl, bool is_force);

public:
	struct TCEntry
	{
		void* head;
		void* tail;
	};

	// ������������������
	static const s32 kMaxNumTransferEntries = 64;

private:
	SizeClass cl_;
	Span empty_; // �Ѿ�ȫ�������ȥ��span
	Span noempty_; // ���ַ����ȥ��span, �����Լ�������
	u64 num_spans_; // empty + noempty��span�ܺ�
	TCEntry tc_slots_[kMaxNumTransferEntries];
	s32 used_slots_; // ʹ�õ�tc_slots��Ŀ
	s32 cache_size_; // ��ǰ������slot��С
	s32 max_cache_size_; // ���������slot��С
	u64 free_obj_num_; // ���ж�������,����span�ڵĶ�������
};
