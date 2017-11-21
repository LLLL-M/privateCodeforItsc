#ifndef __LFQ_H__
#define __LFQ_H__

#ifdef __KERNEL__
#include <asm/string.h>
#else
#include <string.h>
#endif

#ifdef __linux__
#include <sys/errno.h>
#else
#define EWOULDBLOCK 4
#define EINVAL 1
#endif

#include "hik.h"

struct lfq_t
{
	volatile UInt32 rd, wr;

	UInt32 element_size;
	UInt32 queue_size;

	char data[1];
};

static inline void* lfq_reinit(void* handle)
{
	struct lfq_t *queue = handle;
	if (queue)
	{
		queue->rd = 0;
		queue->wr = 0;
		return queue;
	}
	else
		return NULL;

}
static inline UInt32 lfq_calc_total_size(UInt32 element_num, 
		                                      UInt32 element_size) 
{
	UInt32 queue_size = element_num * element_size;
	return (queue_size + offsetof(struct lfq_t, data));
}
static inline void* lfq_init(void* handle, 
                             UInt32 total_size, 
                             UInt32 element_size)
{
	struct lfq_t *queue = handle;
	
	if (queue)
	{
		queue->rd = 0;
		queue->wr = 0;

		queue->element_size = element_size;
		queue->queue_size = (total_size - offsetof(struct lfq_t, data))
								 / element_size;
		queue->queue_size *= element_size;
        return queue;
	}
	else
		return NULL;
}

static inline int lfq_write(void* handle, void* data)
{
	struct lfq_t* queue = handle;
    if (queue)
	{
		volatile register UInt32 wr = queue->wr, next_wr;
			
		next_wr = wr + queue->element_size;
		if (next_wr >= queue->queue_size)
			next_wr = 0;

		if (next_wr == queue->rd)
			return -EWOULDBLOCK;

		memcpy((char*)queue->data + wr, data, queue->element_size);
        queue->wr = next_wr;   
		return 0;
	}
	else
		return -EINVAL;
}

static inline int lfq_read(void* handle, void* data)
{
	struct lfq_t* queue = handle;
	if (queue)
	{
		register UInt32 rd = queue->rd, next_rd;
	
		if (rd == queue->wr)
			return -EWOULDBLOCK;
	
		memcpy(data, (char*)queue->data + rd, queue->element_size);
		next_rd = rd + queue->element_size;
		if (next_rd >= queue->queue_size)
			 next_rd = 0;
        queue->rd = next_rd;
        return 0;
    }
	else
	{
		return -EINVAL;
	}
}



static inline int lfq_element_count(void *handle)
{
    struct lfq_t *queue = handle;
    if (queue)
    {
        volatile register UInt32 rd = queue->rd;
        volatile register UInt32 wr = queue->wr;
        UInt32 size;

        if (rd <= wr)
            size = wr - rd;
        else
            size = queue->queue_size - rd + wr;

        return size / queue->element_size;
    }
    else
        return 0;
}

static inline int lfq_free_count(void* handle)
{
    struct lfq_t* queue = handle;
    if (queue)
    {
        volatile register UInt32 rd = queue->rd;
        volatile register UInt32 wr = queue->wr;
        UInt32 size;

        if (rd <= wr)
            size = wr - rd;
        else
            size = wr + queue->queue_size - rd;

        return ((queue->queue_size - size) / queue->element_size - 1);
    }
    else
        return 0;
}

static inline int lfq_total_count(void* handle)
{
    struct lfq_t* queue = handle;
    if (queue)
        return queue->queue_size / queue->element_size - 1;
    else
        return 0;

}

static inline void* lfq_reserve(void *handle, int *num)
{
    struct lfq_t* queue = handle;
    if (queue)
    {
        volatile register UInt32 rd = queue->rd;
        volatile register UInt32 wr = queue->wr;
        UInt32 size;

        if (rd <= wr)
        {
            size = queue->queue_size - wr;
            if (rd == 0)
                size -= queue->element_size;
        }
        else
            size = rd - wr - queue->element_size;

        *num = size / queue->element_size;

        if (*num > 0)
            return queue->data + wr;
        else
            return NULL;
    }
    else
        return NULL;
}

static inline int lfq_commit(void *handle, int num)
{
    struct lfq_t* queue = handle;
    if (queue)
    {
        volatile register UInt32 wr = queue->wr, next_wr;

        next_wr = wr + num * queue->element_size;
        if (next_wr >= queue->queue_size)
            next_wr -= queue->queue_size;

        queue->wr = next_wr;
        return 0;
    }
    else
        return -EINVAL;
}

static inline int lfq_read_n(void *handle, void *data, int num)
{
    struct lfq_t* queue = handle;
    if (queue)
    {
        register UInt32 rd = queue->rd, next_rd;
        UInt32 size = num * queue->element_size;
        UInt32 size_to_end = queue->queue_size - rd;

        if (size > size_to_end)
        {
            memcpy(data, queue->data + rd, size_to_end);
            data += size_to_end;
            rd = 0;
            size -= size_to_end;
            memcpy(data, queue->data + rd, size);
        }
        else if (size < size_to_end)
        {
            memcpy(data, queue->data + rd, size);
        }
        else // size == size_to_end
        {
            memcpy(data, queue->data + rd, size_to_end);
            rd = 0;
            size -= size_to_end;
        }

        next_rd = rd + size;
        queue->rd = next_rd;
        return 0;
    }
    else
        return -EINVAL;
}
//倒序读取线性队列数据块
static inline int lfq_read_inverted(void* handle, void* data, int back_no)
{
	struct lfq_t* queue = handle;
	if (queue && back_no >= 0)
	{
		register UInt32 rd = queue->rd;
		UInt32 size = back_no * queue->element_size;

		rd = (rd >= size) ? (rd - size) : (rd + queue->queue_size - size);
		memcpy(data, (char*)queue->data + rd, queue->element_size);
		rd += queue->element_size;
		queue->rd = (rd >= queue->queue_size) ? 0 : rd;
        return 0;
    }
	else
	{
		return -EINVAL;
	}
}
//预先读取线性队列数据块而不会改变读的位置rd
static inline void *lfq_read_prefetch(void* handle, int block_no)
{
	struct lfq_t* queue = handle;
	if (queue)
	{
		register UInt32 rd = queue->rd, next_rd;
	
		if (rd == queue->wr)
			return NULL;
	
		next_rd = rd + queue->element_size * block_no;
		if (next_rd >= queue->queue_size)
			next_rd -= queue->queue_size;
        return (void *)(queue->data + next_rd);
    }
	else
	{
		return NULL;
	}
}

//往回查找队列数据块中第block_no个数据块；
//不会改变rd和wr指针的位置；
static inline void *lfq_read_back(void *handle, int block_no)
{
	struct lfq_t* queue = handle;
	if (queue)
	{
		register UInt32 rd = queue->rd, back_rd;
        if(rd <= queue->wr)
        {
            if(rd >= queue->element_size * block_no)
        		back_rd = rd - queue->element_size * block_no;
            else
            {
        		back_rd = rd + queue->queue_size - queue->element_size * block_no;
                //队列中往回第block_no个数据块已经被重新写入；
                if (back_rd < queue->wr)
                    return NULL;
            }
        }
        else
        {
            if(rd >= queue->element_size * block_no)
            {
        		back_rd = rd - queue->element_size * block_no;
                //队列中往回第block_no个数据块已经被重新写入；
                if(back_rd < queue->wr)
                    return NULL;
            }
            //队列中往回第block_no个数据块已经被重新写入；
            else
                return NULL;
        }
        return (void *)(queue->data + back_rd);
    }
	else
	{
		return NULL;
	}
}

#endif //__EB_LFQ_H__
