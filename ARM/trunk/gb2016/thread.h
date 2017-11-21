#ifndef __THREAD_H__
#define	__THREAD_H__

#ifdef __linux__
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#define WINAPI
typedef pthread_t ThreadHandle;
typedef void * ThreadReturn;
#else	//__WIN32__
#include <windows.h>
#include <process.h>
typedef HANDLE ThreadHandle;
typedef unsigned int ThreadReturn;
#endif

#define THREAD_STACK_SIZE	(2 << 20)	//2M
#define INVALID_THREAD_HANDLE 0			//无效的线程句柄

namespace HikThread
{
	class Thread
	{
	private:
		ThreadHandle handle;
        static ThreadReturn WINAPI func(void *arg)
		{
			Thread *th = static_cast<Thread *>(arg);
			th->run(arg);
            return 0;
		}
	public:
		Thread()
		{
			handle = INVALID_THREAD_HANDLE;
		}
        virtual ~Thread()
		{
			stop();
		}
		virtual void run(void *arg) = 0;
		bool start()
		{
			if (handle != INVALID_THREAD_HANDLE)
				return false;
	#ifdef __linux__
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
			if (pthread_create(&handle, &attr, func, this) == 0)
			{
				pthread_detach(handle);
				return true;
			}
			else
			{
				handle = INVALID_THREAD_HANDLE;
				return false;
			}
	#else	//__WIN32__
            handle = (ThreadHandle)_beginthreadex(NULL, THREAD_STACK_SIZE, func, this, 0, 0);
			return (handle != NULL);
	#endif			
		}
		bool isdead()
		{
			if (handle == INVALID_THREAD_HANDLE)
				return true;
#ifdef __linux__
			return (pthread_kill(handle, 0) == ESRCH);
#else
			return (WaitForSingleObject(handle, 0) == WAIT_OBJECT_0);
#endif
		}
		void stop()
		{
			if (handle != INVALID_THREAD_HANDLE)
			{
#ifdef __linux__
				pthread_cancel(handle);
#else	//__WIN32__
				TerminateThread(handle, 0);
				CloseHandle(handle);
#endif
				handle = INVALID_THREAD_HANDLE;
			}
		}
	};	
}

#endif
