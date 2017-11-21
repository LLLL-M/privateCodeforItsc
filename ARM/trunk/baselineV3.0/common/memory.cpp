#include "memory.h"

hik::memory::MemNode *hik::memory::getidle(std::size_t size)
{
	MemNode *prev = nullptr;
	for (MemNode *cur = idle; cur != nullptr; prev = cur, cur = prev->next)
	{
		if (cur->size < size)
			continue;

		std::size_t left = cur->size - size;
		if (left > sizeof(MemNode))
		{
			MemNode *node = reinterpret_cast<MemNode *>((char *)cur + sizeof(MemNode) + size);
			node->next = cur->next;
			node->size = left - sizeof(MemNode);
			if (prev == nullptr)
				idle = node;
			else
				prev->next = node;
			cur->size = size;
		}
		else
		{
			if (prev == nullptr)
				idle = cur->next;
			else
				prev->next = cur->next;
		}
		return cur;
	}
	return nullptr;
}

void hik::memory::putidle(hik::memory::MemNode *node)
{
	auto link = [](MemNode *node, MemNode *cur){
		size_t offset = static_cast<size_t>(cur - node);
		if (offset == sizeof(MemNode) + node->size)
		{
			node->size += sizeof(MemNode) + cur->size;
			node->next = cur->next;
		}
		else
			node->next = cur;
	};
	MemNode *prev = nullptr;
	for (MemNode *cur = idle; cur != nullptr; prev = cur, cur = prev->next)
	{
		if (node > cur)
			continue;
		
		link(node, cur);//与后面的节点连接
		if (prev == nullptr)
			idle = node;
		else
			link(prev, node);//与前面的节点连接
		return;
	}
	if (prev == nullptr)
		idle = node;
	else
		prev->next = node;
}

hik::memory::MemNode *hik::memory::getused(void *p)
{
	if (used == nullptr || p == nullptr)
		return nullptr;
	MemNode *address = reinterpret_cast<MemNode *>(p);
	MemNode *prev = nullptr;
	for (MemNode *cur = used; cur != nullptr; prev = cur, cur = prev->next)
	{
		if (cur + 1 == address)
		{
			if (prev == nullptr)
				used = cur->next;
			else
				prev->next = cur->next;
			return cur;
		}
	}
	return nullptr;
}

void *hik::memory::alloc(std::size_t size)
{
	if (size == 0)
		return nullptr;
	size = pad8byte(size);
	hik::mutex_guard guard(mutex);
	MemNode *node = getidle(size);
	if (node)
	{
		putused(node);
		return reinterpret_cast<void *>(node + 1);
	}

	std::size_t allocsize = BLOCKSIZE;
	while (size + 2 * sizeof(MemNode) > allocsize)
		allocsize += BLOCKSIZE;
	node = reinterpret_cast<MemNode *>(calloc(1, allocsize));
	if (node == nullptr)
		return nullptr;
	node->next = block;
	node->size = allocsize - sizeof(MemNode);
	block = node;

	node++;
	node->size = size;
	putused(node);
	void *ret = reinterpret_cast<void *>(node + 1);

	std::size_t usedsize = size + 2 * sizeof(MemNode);
	if (allocsize - usedsize > sizeof(MemNode))
	{
		node = reinterpret_cast<MemNode *>((char *)block + usedsize);
		node->next = nullptr;
		node->size = allocsize - usedsize - sizeof(MemNode);
		putidle(node);
	}
	
	return ret;
}

void hik::memory::free(void *address)
{
	hik::mutex_guard guard(mutex);
	MemNode *node = getused(address);
	if (node)
	{
		node->next = nullptr;
		putidle(node);
	}
}
