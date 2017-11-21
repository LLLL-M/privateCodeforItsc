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

namespace HikSem
{
	class Sem
	{
	private:
		SemHandle handle;
		bool succ;
		
	public:
		Sem(const int maxval = 0xfff, const int initval = 0)
		{
		#ifdef __linux__
			succ = (sem_init(&handle, 0, initval) == 0);
		#else	//__WIN32__
			handle = CreateSemaphore(0, initval, maxval, 0);
			succ = (handle != 0);
		#endif
		}
        virtual ~Sem()
		{
			if (succ)
			{
				succ = false;
			#ifdef __linux__
				sem_destroy(&handle);
			#else	//__WIN32__
				CloseHandle(handle);
			#endif
			}
		}
		void post()
		{
			if (!succ)
				return;
		#ifdef	__linux__
			sem_post(&handle);
		#else	//__WIN32__
			ReleaseSemaphore(handle, 1, NULL);
		#endif
		}
		bool wait(bool isblock = true)	//默认阻塞等待信号量
		{
			if (!succ)
				return false;
		#ifdef	__linux__
			return (isblock ? (0 == sem_wait(&handle)) : (0 == sem_trywait(&handle)));
		#else	//__WIN32__
			return (WAIT_OBJECT_0 == WaitForSingleObject(handle, isblock ? INFINITE : 0));
		#endif	
		}
	};
}

#endif
