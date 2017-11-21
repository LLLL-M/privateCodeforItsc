#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <list>
#include "thread.h"
#include "common.h"

class Camera : public HikThread::Thread
{
private:
	enum {MAX_CAMERA_NUM = 40, MAX_LANE_NUM = 48,};
	struct TscStream
	{
		int fd = -1;
		char *buf = nullptr;
		int buflen = 2048;
		int offset = 0;
		TscStream() = default;
		TscStream(int _fd, int _buflen = 2048);
		~TscStream();
		bool operator()(TscStream &stream)
		{
			return stream.fd == fd;
		}
	};
	list<TscStream> streams;
	
	TscStream &StreamCreate(int fd);
	void StreamDelete(int fd);
	void StreamDelete(TscStream &stream);
	bool StreamRead(TscStream &stream, int size);
	
	void RecvCameraData(int fd);

public:
	static int epollfd;
	void run(void *arg);
protected:
};

#endif
