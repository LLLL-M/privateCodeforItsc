#pragma once

#ifdef __linux__
#include <pthread.h>
#include <signal.h>
#define WINAPI
typedef pthread_t ThreadHandle;
typedef void * ThreadReturn;
#else	//__WIN32__
#include <windows.h>
#include <process.h>
typedef HANDLE ThreadHandle;
typedef unsigned int ThreadReturn;
#endif
#include <functional>

#define THREAD_STACK_SIZE	(6 << 20)	//6M
#define INVALID_THREAD_HANDLE 0			//invalid handle

namespace hik
{
	class thread
	{
	private:
		ThreadHandle handle;
		std::function<void()> run;
        static ThreadReturn WINAPI func(void *arg)
		{
			thread *th = static_cast<thread *>(arg);
			if (th->run)
				th->run();
			else
				th->Run();
            return 0;
		}
	public:
		thread()
		{
			handle = INVALID_THREAD_HANDLE;
		}
		thread(std::function<void()> f, bool _start = true) : run(f)
		{
			handle = INVALID_THREAD_HANDLE;
			if (_start)
				start();
		}
        virtual ~thread()
		{
			stop();
		}
		virtual void Run() {}

		bool start(std::function<void()> f)
		{
			run = f;
			return start();
		}
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
			bool ret = (pthread_kill(handle, 0) == ESRCH);
#else
			bool ret = (WaitForSingleObject(handle, 0) == WAIT_OBJECT_0);
#endif
			if (ret)
				handle = INVALID_THREAD_HANDLE;
			return ret;
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
