#pragma once

#include "st_common.h"

// ����������һ��ҳ��ռ�
struct Span
{
	PageID start; // ��ʼҳ���
	Length page_num; // ҳ������
	Span* next;  // ˫������
	Span* prev;  // ˫������
	void* objects; // �����С��������ͷ
	u32 refcount; // �Ѿ�����Ķ�������
	SizeClass sizeclass; // С����������class, ��������Ϊ0
	//u32 location : 2; // ��ʱ����
	//u32 sample : 1;

	// Span����λ�ã���ʱ�ò���
	//enum { IN_USE, ON_NORMAL_FREELIST, ON_RETURNED_FREELIST };
};

// ���������һ��span, span�����ҳ�����ⲿ�����,spanֻ�������
Span* NewSpan(PageID p, Length len);
void DeleteSpan(Span* span);

// span˫��������ش���
void SpanDoubleListInit(Span* list);

// ��������ɾ��һ��Span
void SpanDoubleListRemove(Span* span);

// �ж������Ƿ�Ϊ��
inline bool SpanDoubleListIsEmpty(const Span* list)
{
	return list->next == list;
}

// ������ǰ�����һ��Span
void SpanDoubleListPrepend(Span* list, Span* span);

// ��ȡ������
s32 SpanDoubleListLength(const Span* list);
