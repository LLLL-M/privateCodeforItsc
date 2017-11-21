#ifndef __STREAM_H__
#define __STREAM_H__

#include <stdlib.h>
#include <string.h>
#include "list.h"

static struct list_head gStreamHead = LIST_HEAD_INIT(gStreamHead);
static int epollfd = -1;

typedef struct stream_info
{
	struct list_head node;
	int fd;
	char *buf;
	int offset;
} Stream;

static inline void StreamClose(int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}

static inline void StreamReset(Stream *stream)
{
	if (stream == NULL)
		return;
	memset(stream->buf, 0, 1024);
	stream->offset = 0;
}

static inline void StreamDelete(Stream *stream)
{
	if (stream != NULL)
	{
		StreamClose(stream->fd);
		list_del(&stream->node);
		if (stream->buf != NULL)
			free(stream->buf);
		free(stream);
	}
}

static inline void StreamFullDeal()
{
	struct list_head *node = NULL;
	
	if (list_count(&gStreamHead) < 24)
		return;
	//如果流链表中超过24个流，则删除最开始插入的流
	node = gStreamHead.next;
	if (node != NULL)
		StreamDelete((Stream *)node);
}

static inline Stream *StreamFind(int fd)
{
	Stream *stream = NULL;
	struct list_head *node;
	list_for_each(node, &gStreamHead)
	{
		stream = (Stream *)node;
		if (stream->fd == fd)
			return stream;
	}
	return NULL;
}

static inline Stream *StreamCreate(int fd)
{
	Stream *stream = StreamFind(fd);
	
	if (stream != NULL)
		return stream;
	stream = calloc(1, sizeof(Stream));
	if (stream != NULL)
	{
		stream->buf = calloc(1, 1024);
		if (stream->buf != NULL)
		{
			StreamFullDeal();
			list_add_tail(&stream->node, &gStreamHead);
			stream->fd = fd;
			stream->offset = 0;
		}
		else
		{
			free(stream);
			stream = NULL;
		}
	}
	return stream;
}

static inline Boolean StreamRead(Stream *stream, int size)
{
	int n = 0;
	Boolean ret = TRUE;
	
	if (stream->offset < size)
	{
		n = read(stream->fd, stream->buf + stream->offset, size - stream->offset);
		if (n == -1)
		{
			if (!(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			{
				StreamDelete(stream);
				//perror("tcp stream occur error");
			}
			ret = FALSE;
		}
		else if (n == 0)
		{	//说明对方tcp连接已经关闭
			StreamDelete(stream);
			ret = FALSE;
			//INFO("tcp stream has been closed!");
		}
		else
		{
			stream->offset += n;
			ret = (stream->offset == size) ? TRUE : FALSE;
		}	
	}
	return ret;
}

#endif
