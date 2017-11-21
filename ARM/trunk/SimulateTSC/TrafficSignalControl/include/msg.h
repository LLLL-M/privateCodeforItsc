#ifndef __MSG_H__
#define __MSG_H__

#include <queue>
#include <ctime>
#include <mutex>
#include "sem.h"

namespace HikMsg
{
	using namespace HikSem;
	template<class T>
	class Msg: public Sem
	{
	private:
		typedef std::queue<T> MsgQueue;
		static Manage<MsgQueue *> manage;
		MsgQueue *msgqueue;
		std::string msgname;
		std::mutex mutexlock;
	public:
		Msg(const char *name) : Sem(name), msgqueue(NULL), msgname(name)
		{
			if (!manage.get(name, msgqueue))
			{
				msgqueue = new MsgQueue;
				if (msgqueue != NULL)
					manage.add(name, msgqueue);
			}
		}
		~Msg()
		{
			if (msgqueue != NULL)
			{
				if (manage.del(msgname))
					delete msgqueue;
				msgqueue = NULL;
			}
		}
		bool recv(T & data, bool isblock = true)	//默认阻塞接收消息
		{
			bool ret;
            if (msgqueue == NULL || !wait(isblock))
				return false;
			mutexlock.lock();
			ret = !msgqueue->empty();
			if (ret)
			{
				data = msgqueue->front();
				msgqueue->pop();
			}
			mutexlock.unlock();
			return ret;
		}
		void send(T & data)
		{
			if (msgqueue == NULL)
				return;
			mutexlock.lock();
			msgqueue->push(data);
			mutexlock.unlock();
			post();
		}
	};
}

#endif
