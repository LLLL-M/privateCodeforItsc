#ifndef __SEM_H__
#define __SEM_H__

#ifdef __linux__
#include <semaphore.h>
typedef sem_t SemHandle;
#else	//__WIN32__
#include <windows.h>
#include <process.h>
typedef HANDLE SemHandle;
#endif
#include "manage.h"

namespace HikSem
{
	using HikManage::Manage;
	class Sem
	{
	private:
		static Manage<SemHandle *> manage;
		SemHandle *handle;
		std::string semname;
		bool succ;
	public:
        Sem(const char *name, const int initval = 0, const int maxval = 0xfff) : handle(NULL), semname(name), succ(true)
		{
			if (!manage.get(name, handle))
			{
				handle = new SemHandle;
			#ifdef __linux__
				succ = (sem_init(handle, 0, initval) == 0);
			#else	//__WIN32__
                *handle = CreateSemaphore(NULL, initval, maxval, 0);
				succ = (*handle != NULL);
			#endif
				if (succ)
					manage.add(name, handle);
			}
		}
        virtual ~Sem()
		{
			if (succ)
			{
				succ = false;
				if (manage.del(semname))
				{
	#ifdef __linux__
					sem_destroy(handle);
	#else	//__WIN32__
					CloseHandle(*handle);
	#endif
					delete handle;
				}
			}
		}
		void post()
		{
			if (!succ)
				return;
#ifdef	__linux__
			sem_post(handle);
#else	//__WIN32__
			ReleaseSemaphore(*handle, 1, NULL);
#endif
		}
		bool wait(bool isblock = true)	//默认阻塞等待信号量
		{
			if (!succ)
				return false;
#ifdef	__linux__
			return (isblock ? (0 == sem_wait(handle)) : (0 == sem_trywait(handle)));
#else	//__WIN32__
			return (WAIT_OBJECT_0 == WaitForSingleObject(*handle, isblock ? INFINITE : 0));
#endif	
		}
	};
}

#endif
