#pragma once

#include "span.h"
#include "size_map.h"
#include "central_free_list.h"
#include "thread_cache.h"
#include "page_heap.h"
#include "meta_allocator.h"
#include "span.h"

#include "st_common.h"

class CentralFreeList;
struct Span;

using SpanAllocator = MetaAllocator<Span>;
using PageMapNodeAllocator = PageMapType::NodeAllocator;
using PageMapLeafAllocator = PageMapType::LeafAllocator;

// �ڴ������ͳ����Ϣ
struct STMallInfo
{
	u64 total_alloc_size;
	u64 meta_alloc_size; // metaϵͳ������ڴ���Ŀ
	STMallInfo()
	{
		total_alloc_size = 0;
		meta_alloc_size = 0;
	}
};

// ���߳��ڴ������
class STMalloc
{
public:
	STMalloc();
	~STMalloc();

public:
	s32 Init(AllocPagesPtr alloc_ptr, FreePagesPtr free_ptr);
	s32 Resume(AllocPagesPtr alloc_ptr, FreePagesPtr free_ptr);

	void Dump();

public:
	bool is_inited() { return is_inited_; }
	SizeMap& size_map() { return size_map_; }
	ThreadCache& thread_cache() { return thread_cache_; }
	CentralFreeList* center_lists() { return central_lists_; }

	PageHeap& page_heap() { return page_heap_; }
	STMallInfo& mall_info() { return mall_info_; }
	SpanAllocator& span_allocator() { return span_allocator_; }
	// page map����ڵ����õķ�����
	PageMapNodeAllocator& page_map_node_allocator() { return page_map_node_allocator_; }
	PageMapLeafAllocator& page_map_leaf_allocator() { return page_map_leaf_allocator_; }

private:
	bool is_inited_; // �Ƿ�����˳�ʼ��
	SizeMap size_map_;
	CentralFreeList central_lists_[kNumClasses];
	ThreadCache thread_cache_;
	PageHeap page_heap_;
	STMallInfo mall_info_;

	// metadata ��������ֻ���䲻�ͷ�
	SpanAllocator span_allocator_;
	PageMapNodeAllocator  page_map_node_allocator_;
	PageMapLeafAllocator page_map_leaf_allocator_;
};

extern STMalloc* g_st_malloc;

void* st_malloc(size_t size);
void st_free(void* ptr);


