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
    unsigned int refcount; // �Ѿ�����Ķ�������
    SizeClass sizeclass; // С����������class, ��������Ϊ0
    //unsigned int location : 2; // ��ʱ����
    //unsigned int sample : 1;

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
int SpanDoubleListLength(const Span* list);
