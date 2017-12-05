#ifndef __SEM_H__
#define __SEM_H__

#ifdef __linux__
#include <semaphore.h>
#include <time.h>
typedef sem_t SemHandle;
#else	//__WIN32__
#include <windows.h>
#include <process.h>
typedef HANDLE SemHandle;
#endif

namespace hik
{
	class sem
	{
	private:
		SemHandle handle;
		bool succ;
		
	public:
		sem(const int maxval = 0xfff, const int initval = 0)
		{
		#ifdef __linux__
			succ = (sem_init(&handle, 0, initval) == 0);
		#else	//__WIN32__
			handle = CreateSemaphore(0, initval, maxval, 0);
			succ = (handle != 0);
		#endif
		}
        ~sem()
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
		bool wait()
		{
			if (!succ)
				return false;
		#ifdef	__linux__
			return (0 == sem_wait(&handle));
		#else	//__WIN32__
			return (WAIT_OBJECT_0 == WaitForSingleObject(handle, INFINITE));
		#endif
		}
		bool waitfor(unsigned int msec)
		{
			if (!succ || msec == 0)
				return false;
		#ifdef __linux__
			struct timespec ts;
			if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
				return false;
			ts.tv_sec += msec / 1000;
			ts.tv_nsec += (msec % 1000) * 1000000;
			return (0 == sem_timedwait(&handle, &ts));
		#else	//__WIN32__
			return (WAIT_OBJECT_0 == WaitForSingleObject(handle, msec));
		#endif
		}
		bool trywait()
		{
			if (!succ)
				return false;
		#ifdef	__linux__
			return (0 == sem_trywait(&handle));
		#else	//__WIN32__
			return (WAIT_OBJECT_0 == WaitForSingleObject(handle, 0));
		#endif
		}
		int getval()
		{
		#ifdef __linux__
			int v = 0;
			sem_getvalue(&handle, &v);
			return v;
		#else
			return 0;
		#endif
		}
	};
}

#endif
