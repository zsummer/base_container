#pragma once
#include "link_list.h"
#include "st_common.h"

class FreeList
{
public:
	FreeList()
	{
		list_ = NULL;
		length_ = 0;
		lowater_ = 0;
		max_length_ = kDefaultTransferNumObjects; // ֱ��������128�� ����������
		length_overages_ = 0;
	}
	void Init()
	{
		list_ = NULL;
		length_ = 0;
		lowater_ = 0;
		max_length_ = kDefaultTransferNumObjects; // ֱ��������128�� ����������
		length_overages_ = 0;
	}

	int length() const { return length_; }
	int max_length() { return max_length_; }
	void set_max_length(int new_max)
	{
		max_length_ = new_max;
	}

	unsigned int length_overages() const
	{
		return length_overages_;
	}

	void set_length_overages(int new_count)
	{
		length_overages_ = new_count;
	}

	bool empty() const
	{
		return (list_ == NULL);
	}

	unsigned int lowwater_mark() { return lowater_; }
	void clear_lowwater_mark() { lowater_ = length_; }

	void Push(void* ptr)
	{
		SLL_Push(&list_, ptr);
		length_++;
	}

	void* Pop()
	{
		if (list_ == NULL)
		{
			return NULL;
		}
		length_--;
		if (length_ < lowater_)
		{
			lowater_ = length_;
		}
		return SLL_Pop(&list_);
	}

	void* Next()
	{
		return SLL_Next(&list_);
	}

	void PushRange(int N, void* start, void* end)
	{
		SLL_PushRange(&list_, start, end);
		length_ += N;
	}

	void PopRange(int num, void** start, void ** end)
	{
		SLL_PopRange(&list_, num, start, end);
		//ASSERT(length_ >= N);
		length_ -= num;
		if (length_ < lowater_)
		{
			lowater_ = length_;
		}
	}

private:
	void* list_; // ����ͷ
	int length_; // ��ǰ����
	int max_length_; // ��̬��󳤶�
	int lowater_; // ��ˮ�߳���
	int length_overages_; // ͳ�����ͷ�ʱlength_ > max_length_ �Ĵ���
};
