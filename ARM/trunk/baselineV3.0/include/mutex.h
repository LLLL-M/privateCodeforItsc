#pragma once

#ifdef __linux__
#include <pthread.h>
typedef pthread_mutex_t MutexHandle;
#else	//__WIN32__
#include <windows.h>
typedef HANDLE MutexHandle;
#endif

namespace hik
{
	class mutex
	{
	private:
		MutexHandle handle;
		bool succ;
		
	public:
		mutex()
		{
		#ifdef __linux__
			succ = (pthread_mutex_init(&handle, NULL) == 0);
		#else	//__WIN32__
			handle = CreateMutex(NULL, FALSE, NULL);
			succ = (handle != 0);
		#endif
		}
        ~mutex()
		{
			if (succ)
			{
				succ = false;
			#ifdef __linux__
				pthread_mutex_destroy(&handle);
			#else	//__WIN32__
				CloseHandle(handle);
			#endif
			}
		}
		void lock()
		{
			if (!succ)
				return;
		#ifdef	__linux__
			pthread_mutex_lock(&handle);
		#else	//__WIN32__
			WaitForSingleObject(handle, INFINITE);
		#endif
		}
		void unlock()
		{
			if (!succ)
				return;
		#ifdef	__linux__
			pthread_mutex_unlock(&handle);
		#else	//__WIN32__
			ReleaseMutex(handle);
		#endif
		}
		bool try_lock()
		{
			if (!succ)
				return false;
		#ifdef	__linux__
			return 0 == pthread_mutex_trylock(&handle);
		#else	//__WIN32__
			return WAIT_OBJECT_0 == WaitForSingleObject(handle, 0);
		#endif
		}
	};

	class mutex_guard
	{
	private:
		mutex &mtx;
	public:
		mutex_guard(mutex &m) : mtx(m)
		{
			mtx.lock();
		}
		~mutex_guard()
		{
			mtx.unlock();
		}
	};
}
