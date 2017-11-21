#pragma once

#include <cstdlib>
#include "mutex.h"

namespace hik
{
	class memory
	{
	private:
		enum { BLOCKSIZE = 4096 };
		struct alignas(8) MemNode
		{
			MemNode *next;
			std::size_t size;
		};
		MemNode *used;	//已使用的内存的头结点
		MemNode *idle;	//空闲的内存的头结点
		MemNode *block;	//所申请的内存块的头结点

		hik::mutex mutex;

		std::size_t pad8byte(std::size_t size) const noexcept
		{
			std::size_t left = size & 0x7;
			return (left) ? (size + 8 - left) : size;
		}

		MemNode *getidle(std::size_t size);

		void putidle(MemNode *node);

		MemNode *getused(void *p);

		void putused(MemNode *node)
		{
			node->next = used;
			used = node;
		}

	public:
		memory()
		{
			used = nullptr;
			idle = nullptr;
			block = nullptr;
		}

		~memory()
		{
			MemNode *next = nullptr;
			while (block)
			{
				next = block->next;
				std::free(block);
				block = next;
			}
		}
		
		void *alloc(std::size_t size);

		void free(void *address);
	};
}