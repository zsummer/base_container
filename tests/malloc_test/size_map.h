#pragma once


#include "st_common.h"

// ���ڴ�ķ���ߴ磬 �õĴ���ڴ�������������size class ����Ŀ
u32 AlignmentForSize(size_t size);

// size-class��ӳ���ϵ��Ϣ
class SizeMap
{
public:
    SizeMap();
    // ��ʼ�����е�ӳ���ϵ
    s32 Init();

    void Dump();

    // ����size ��ȡ����Ӧ���ⲿClass����
    inline s32 SizeClass(s32 size)
    {
        return class_array_[ClassArrayIndex(size)];
    }
    inline size_t ByteSizeForClass(size_t cl)
    {
        return class_to_size_[cl];
    }
    inline size_t class_to_pages(size_t cl)
    {
        return class_to_pages_[cl];
    }
    inline s32 num_objects_to_move(size_t cl)
    {
        return num_objects_to_move_[cl];
    }

private:
    s32 num_objects_to_move_[kNumClasses];
    // С�ߴ���������С
    static const s32 kMaxSmallSize = 1024;
    static const size_t kClassArraySize = ((kMaxSize + 127 + (120 << 7)) >> 7) + 1;

    u8 class_array_[kClassArraySize];

    static inline size_t ClassArrayIndex(s32 s)
    {
        /*if (0 > s || s > kMaxSize)
        {
            return 0;
        }
        */

        if (s <= kMaxSmallSize) // 1024һ�¡�����8�ֽڶ���
        {
            return (static_cast<u32>(s) + 7) >> 3;
        }
        else // 1024 ��256k֮�䣬����128����
        {
            return (static_cast<u32>(s) + 127 + (120 << 7)) >> 7;
        }
    }

    // ����ĳ����Сһ�������ƶ�������
    size_t NumMoveSize(size_t size);

    // ��size class ����class ���Դ洢�����ߴ�ӳ��
    size_t class_to_size_[kNumClasses];

    // ��size class ����class ÿ����Ҫ�����Page������ӳ��
    size_t class_to_pages_[kNumClasses];
};
