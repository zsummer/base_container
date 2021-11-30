#pragma once

#include <stddef.h>
#include <string.h>
#include "meta_allocator.h"

// raidx tree, ʵ���˴�1����3�����ַ�ʽ

template<s32 BITS>
class PageMap1
{
public:
	explicit PageMap1(void* (*allocator)(size_t))
	{
		array_ = reinterpret_cast<void**>((*allocator)(sizeof(void*) << BITS));
		memset(array_, 0, sizeof(void*) << BITS);
	}

	bool Ensure(u64 x, size_t n)
	{
		return n <= LENGTH - x;
	}

	void PreallocateMoreMemory() {}

	void* get(u64 k) const
	{
		if ((k >> BITS) > 0)
		{
			return NULL;
		}
		return array_[k];
	}

	void set(u64 k, void* v)
	{
		array_[k] = v;
	}

	void* Next(u64 k) const
	{
		while (k < (1 << BITS))
		{
			if (array_[k] != NULL)
			{
				return array_[k];
			}
			k++;
		}
		return NULL;
	}

private:
	static const s32 LENGTH = 1 << BITS;
	void** array_;
	//void* (*allocator_)(size_t); // ������
};

template<s32 BITS>
class PageMap2
{
public:
	explicit PageMap2(void* (*allocator)(size_t))
	{
		allocator_ = allocator;
		memset(root_, 0, sizeof(root_));
	}

	void* get(u64 k) const
	{
		// ��λ
		const u64 i1 = k >> LEAF_BITS;
		// ��λ
		const u64 i2 = k & (LEAF_LENGTH - 1);
		if ((k >> BITS) > 0 || root_[i1] == NULL)
		{
			return NULL;
		}
		return root_[i1]->values[i2];
	}

	void set(u64 k, void* v)
	{
		const u64 i1 = k >> LEAF_BITS;
		const u64 i2 = k & (LEAF_LENGTH - 1);
		if (i1 >= ROOT_LENGTH || root_[i1] == NULL)
		{
			// �˴�Ӧ��error log
			return;
		}
		root_[i1]->values[i2] = v;
	}

	bool Ensure(u64 start, size_t n)
	{
		for (u64 key = start; key < start + n - 1; )
		{
			const u64 i1 = key >> LEAF_BITS;
			if (i1 >= ROOT_LENGTH)
			{
				return false;
			}
			if (root_[i1] == NULL)
			{
				Leaf* leaf = reinterpret_cast<Leaf*> ((*allocator_)(sizeof(Leaf)));
				if (leaf == NULL)
				{
					return false;
				}
				memset(leaf, 0, sizeof(*leaf));
				root_[i1] = leaf;
			}

			key = ((key >> LEAF_BITS) + 1) << LEAF_BITS;
		}
		return true;
	}

	void PreallocateMoreMemory()
	{
		Ensure(0, 1 << BITS);
	}

	void* Next(u64 k) const
	{
		while (k < (1 << BITS))
		{
			const u64 i1 = k >> LEAF_BITS;
			Leaf* leaf = root_[i1];
			if (leaf != NULL)
			{
				for (u64 i2 = k & (LEAF_LENGTH - 1); i2 < LEAF_LENGTH; i2++)
				{
					if (leaf->values[i2] != NULL)
					{
						return leaf->values[i2];
					}
				}
			}
			k = (i1 + 1) << LEAF_BITS;
		}
		return NULL;
	}

private:
	static const s32 ROOT_BITS = 5; // ��һ����ռBIT��
	static const s32 ROOT_LENGTH = 1 << ROOT_BITS;

	static const s32  LEAF_BITS = BITS - ROOT_BITS;
	static const s32 LEAF_LENGTH = 1 << LEAF_BITS;

	struct Leaf
	{
		void* values[LEAF_LENGTH];
	};
	Leaf* root_[ROOT_LENGTH]; // ���ĵ�һ��
	void* (*allocator_)(size_t); // ������
};


template <s32 BITS>
class PageMap3
{
public:
	static const s32 INTERIOR_BITS = (BITS + 2) / 3; // �ڲ��ڵ���ռBITS��Ŀ
	static const s32 INTERIOR_LENGTH = 1 << INTERIOR_BITS;

	static const s32 LEAF_BITS = BITS - 2 * INTERIOR_BITS; // Ҷ�ڵ���ռ��BITS��Ŀ
	static const s32 LEAF_LENGTH = 1 << LEAF_BITS;

public:
	// �м�ڵ�
	struct Node
	{
		Node* ptrs[INTERIOR_LENGTH];
	};

	struct Leaf
	{
		void* values[LEAF_LENGTH];
	};

	typedef MetaAllocator<Node> NodeAllocator;
	typedef MetaAllocator<Leaf> LeafAllocator;

	explicit PageMap3()
	{
		root_ = NULL;// NewNode(); // һ��ʼֻ�������ڵ�
		node_allocator_ = NULL;
		leaf_allocator_ = NULL;
	}

	s32 Init(NodeAllocator* node_allocator, LeafAllocator* leaf_allocator)
	{
		if (NULL == node_allocator || NULL == leaf_allocator)
		{
			return ST_ALLOC_ERROR_INVALID_PARAMS;
		}
		node_allocator_ = node_allocator;
		leaf_allocator_ = leaf_allocator;

		root_ = NewNode(); // һ��ʼֻ�������ڵ�
		return 0;
	}

public:
	Node* NewNode()
	{
		Node* result = node_allocator_->New();
		if (result != NULL)
		{
			memset(result, 0, sizeof(*result));
		}
		return result;
	}

	void* get(u64 k) const
	{
		const u64 i1 = k >> (LEAF_BITS + INTERIOR_BITS);
		const u64 i2 = (k >> LEAF_BITS)  & (INTERIOR_LENGTH - 1);
		const  u64 i3 = k & (LEAF_LENGTH - 1);

		// k ���ڵ�ַ��Χ����k��Ӧ��ַ��δ������
		if ((k >> BITS) > 0 || root_->ptrs[i1] == NULL || root_->ptrs[i1]->ptrs[i2] == NULL)
		{
			return NULL;
		}
		return reinterpret_cast<Leaf*>(root_->ptrs[i1]->ptrs[i2])->values[i3];
	}

	void set(u64 k, void* v)
	{
		const u64 i1 = k >> (LEAF_BITS + INTERIOR_BITS);
		const u64 i2 = (k >> LEAF_BITS)  & (INTERIOR_LENGTH - 1);
		const  u64 i3 = k & (LEAF_LENGTH - 1);
		reinterpret_cast<Leaf*>(root_->ptrs[i1]->ptrs[i2])->values[i3] = v;
	}

	bool Reserve(u64 start, size_t n)
	{
		for (u64 key = start; key <= (start + n - 1);)
		{
			const u64 i1 = key >> (LEAF_BITS + INTERIOR_BITS);
			const u64 i2 = (key >> LEAF_BITS) & (INTERIOR_LENGTH - 1);

			// ��ֹ���
			if (i1 >= INTERIOR_LENGTH || i2 >= INTERIOR_LENGTH)
			{
				return false;
			}
			// �м�ڵ㲻���ڣ�����
			if (NULL == root_->ptrs[i1])
			{
				Node* node = NewNode();
				if (NULL == node)
				{
					return false;
				}
				root_->ptrs[i1] = node;
			}

			// Ҷ�ӽڵ㲻���ڣ�����
			if (NULL == root_->ptrs[i1]->ptrs[i2])
			{
				Leaf* leaf = leaf_allocator_->New();
				if (NULL == leaf)
				{
					return false;
				}
				memset(leaf, 0, sizeof(*leaf));
				root_->ptrs[i1]->ptrs[i2] = reinterpret_cast<Node*>(leaf);
			}
			// ÿ��ǰ��һ��LEAF��λ
			key = ((key >> LEAF_BITS) + 1) << LEAF_BITS;
		}
		return true;
	}

	void* Next(u64 k) const
	{
		while (k < (1ull << BITS))
		{
			const u64 i1 = k >> (LEAF_BITS + INTERIOR_BITS);
			const u64 i2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1);

			if (root_->ptrs[i1] == NULL) // ÿ������һ��middle node
			{
				k = (i1 + 1) << (LEAF_BITS + INTERIOR_BITS);
			}
			else
			{
				Leaf* leaf = reinterpret_cast<Leaf*>(root_->ptrs[i1]->ptrs[i2]);
				if (leaf != NULL)
				{
					for (u64 i3 = (k & (LEAF_LENGTH - 1)); i3 < LEAF_LENGTH; ++i3)
					{
						if (leaf->values[i3] != NULL)
						{
							return leaf->values[i3];
						}
					}
				}
				k = ((k >> LEAF_BITS) + 1) << LEAF_BITS;
			}
		}
		return NULL;
	}

private:
	Node* root_; // radix tree�ĸ��ڵ� �����������������radix treeʵ��
	NodeAllocator* node_allocator_;
	LeafAllocator* leaf_allocator_;
};