#ifndef __LOCK_H__
#define __LOCK_H__

#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>
typedef pthread_rwlock_t SRWlock;
#else	//__WIN32__
#include <windows.h>
#include <process.h>
//#pragma comment(lib,"kernel32.lib")
typedef SRWLOCK SRWlock;
#endif

namespace HikLock
{
	class RWlock
	{
	private:
		SRWlock rwlock;
	public:
		RWlock()
		{
#ifdef __linux__
			pthread_rwlock_init(&rwlock, NULL);
#else	//__WIN32__
			InitializeSRWLock(&rwlock);
#endif
		}
		~RWlock()
		{
#ifdef __linux__
			pthread_rwlock_destroy(&rwlock);
#endif
		}
		void r_lock()
		{
#ifdef __linux__
			pthread_rwlock_rdlock(&rwlock);
#else
			AcquireSRWLockShared(&rwlock);
#endif
		}
		void r_unlock()
		{
#ifdef __linux__
			pthread_rwlock_unlock(&rwlock);
#else
			ReleaseSRWLockShared(&rwlock);
#endif
		}
		void w_lock()
		{
#ifdef __linux__
			pthread_rwlock_wrlock(&rwlock);
#else
			AcquireSRWLockExclusive(&rwlock);
#endif
		}
		void w_unlock()
		{
#ifdef __linux__
			pthread_rwlock_unlock(&rwlock);
#else
			ReleaseSRWLockExclusive(&rwlock);
#endif
		}
	};
}

#endif
