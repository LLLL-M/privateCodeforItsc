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
#ifndef EWOULDBLOCK
#define EWOULDBLOCK 4
#endif
#ifndef EINVAL
#define EINVAL 1
#endif
#endif

#include "hik.h"
#include "manage.h"

namespace HikLfq
{
	using HikManage::Manage;
	struct lfq_t
	{
		volatile UInt32 rd, wr;

		UInt32 element_size;
		UInt32 queue_size;

		char data[1];
	};
	class Lfq
	{
	private:
		static Manage<lfq_t *> manage;
		lfq_t *handle;
		std::string lfqname;
	public:
		Lfq() { handle = NULL; }
		Lfq(const char *name, UInt32 total_size = 0, UInt32 element_size = 0) : handle(NULL), lfqname(name)
		{
			if (!manage.get(name, handle))
			{
				if (lfq_init(total_size, element_size))
					manage.add(name, handle);
			}
		}
		~Lfq()
		{
			if (handle != NULL)
			{
				if (manage.del(lfqname))
					delete [] handle;
				handle = NULL;
			}
		}

		bool lfq_init(UInt32 total_size, UInt32 element_size)
		{		
			if (handle != NULL || total_size == 0 || element_size == 0)
				return false;
			handle = (lfq_t *) new char[total_size];
			if (handle)
			{
				handle->rd = 0;
				handle->wr = 0;

				handle->element_size = element_size;
				handle->queue_size = (total_size - offsetof(struct lfq_t, data))
										 / element_size;
				handle->queue_size *= element_size;
				return true;
			}
			else
			{
				ERR("init lfq fail, because memory isn't enough!");
				return false;
			}
		}
		
		bool lfq_reinit()
		{
			if (handle)
			{
				handle->rd = 0;
				handle->wr = 0;
				return true;
			}
			else
				return false;
		}
		
		UInt32 lfq_calc_total_size(UInt32 element_num, UInt32 element_size) 
		{
			UInt32 queue_size = element_num * element_size;
			return (queue_size + offsetof(struct lfq_t, data));
		}
		
		int lfq_write(void* data)
		{
			if (handle)
			{
				volatile register UInt32 wr = handle->wr, next_wr;
					
				next_wr = wr + handle->element_size;
				if (next_wr >= handle->queue_size)
					next_wr = 0;

				if (next_wr == handle->rd)
					return -EWOULDBLOCK;

				memcpy((char*)handle->data + wr, data, handle->element_size);
				handle->wr = next_wr;   
				return 0;
			}
			else
				return -EINVAL;
		}

		int lfq_read(void* data)
		{
			if (handle)
			{
				register UInt32 rd = handle->rd, next_rd;
			
				if (rd == handle->wr)
					return -EWOULDBLOCK;
			
				memcpy(data, (char*)handle->data + rd, handle->element_size);
				next_rd = rd + handle->element_size;
				if (next_rd >= handle->queue_size)
					 next_rd = 0;
				handle->rd = next_rd;
				return 0;
			}
			else
			{
				return -EINVAL;
			}
		}

        int lfq_read_back(void* data)
        {
            if (handle)
            {
                register UInt32 rd = handle->rd, prev_rd;

                prev_rd = rd - handle->element_size;
                if (prev_rd <= 0)
                    prev_rd += handle->queue_size;
                rd = prev_rd;
                handle->rd = prev_rd;
                memcpy(data, (char*)handle->data + rd, handle->element_size);
                return 0;
            }
            else
            {
                return -EINVAL;
            }
        }

		int lfq_element_count()
		{
			if (handle)
			{
				volatile register UInt32 rd = handle->rd;
				volatile register UInt32 wr = handle->wr;
				UInt32 size;

				if (rd <= wr)
					size = wr - rd;
				else
					size = handle->queue_size - rd + wr;

				return size / handle->element_size;
			}
			else
				return 0;
		}

		int lfq_free_count()
		{
			if (handle)
			{
				volatile register UInt32 rd = handle->rd;
				volatile register UInt32 wr = handle->wr;
				UInt32 size;

				if (rd <= wr)
					size = wr - rd;
				else
					size = wr + handle->queue_size - rd;

				return ((handle->queue_size - size) / handle->element_size - 1);
			}
			else
				return 0;
		}

		int lfq_total_count()
		{
			if (handle)
				return handle->queue_size / handle->element_size - 1;
			else
				return 0;

		}
#if 0
		void *lfq_reserve(int *num)
		{
			if (handle)
			{
				volatile register UInt32 rd = handle->rd;
				volatile register UInt32 wr = handle->wr;
				UInt32 size;

				if (rd <= wr)
				{
					size = handle->queue_size - wr;
					if (rd == 0)
						size -= handle->element_size;
				}
				else
					size = rd - wr - handle->element_size;

				*num = size / handle->element_size;

				if (*num > 0)
					return handle->data + wr;
				else
					return NULL;
			}
			else
				return NULL;
		}

		int lfq_commit(int num)
		{
			if (handle)
			{
				volatile register UInt32 wr = handle->wr, next_wr;

				next_wr = wr + num * handle->element_size;
				if (next_wr >= handle->queue_size)
					next_wr -= handle->queue_size;

				handle->wr = next_wr;
				return 0;
			}
			else
				return -EINVAL;
		}

		int lfq_read_n(void *data, int num)
		{
			if (handle)
			{
				register UInt32 rd = handle->rd, next_rd;
				UInt32 size = num * handle->element_size;
				UInt32 size_to_end = handle->queue_size - rd;

				if (size > size_to_end)
				{
					memcpy(data, handle->data + rd, size_to_end);
					data += size_to_end;
					rd = 0;
					size -= size_to_end;
					memcpy(data, handle->data + rd, size);
				}
				else if (size < size_to_end)
				{
					memcpy(data, handle->data + rd, size);
				}
				else // size == size_to_end
				{
					memcpy(data, handle->data + rd, size_to_end);
					rd = 0;
					size -= size_to_end;
				}

				next_rd = rd + size;
				handle->rd = next_rd;
				return 0;
			}
			else
				return -EINVAL;
		}
#endif
        //倒序读取线性队列数据块
        int lfq_read_inverted(void* data, int back_no)
        {
            struct lfq_t* queue = handle;
            if (queue && back_no >= 0)
            {
                register UInt32 rd = queue->rd, prev_rd;
                UInt32 size = back_no * queue->element_size;

                prev_rd = (rd >= size) ? (rd - size) : (rd + queue->queue_size - size);
                memcpy(data, (char*)queue->data + prev_rd, queue->element_size);
                prev_rd += queue->element_size;
                queue->rd = (prev_rd >= queue->queue_size) ? 0 : prev_rd;
                return 0;
            }
            else
            {
                return -EINVAL;
            }
        }

		//预先读取线性队列数据块而不会改变读的位置rd
		void *lfq_read_prefetch(int block_no)
		{
			if (handle)
			{
				register UInt32 rd = handle->rd, next_rd;
			
				if (rd == handle->wr)
					return NULL;
			
				next_rd = rd + handle->element_size * block_no;
				if (next_rd >= handle->queue_size)
					next_rd -= handle->queue_size;
				return (void *)(handle->data + next_rd);
			}
			else
			{
				return NULL;
			}
		}
	};
}

#endif //__EB_LFQ_H__
