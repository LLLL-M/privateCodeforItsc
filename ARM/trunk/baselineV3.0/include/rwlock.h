_Pragma("once")

#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>
typedef pthread_rwlock_t SRWlock;
#else	//__WIN32__
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 //支持window7
#endif
#include <windows.h>
#include <process.h>
//#pragma comment(lib,"kernel32.lib")
typedef SRWLOCK SRWlock;
#endif

namespace hik
{
	class rwlock
	{
	private:
		SRWlock lock;
	public:
		rwlock()
		{
#ifdef __linux__
			pthread_rwlock_init(&lock, NULL);
#else	//__WIN32__
			InitializeSRWLock(&lock);
#endif
		}
		~rwlock()
		{
#ifdef __linux__
			pthread_rwlock_destroy(&lock);
#endif
		}
		void r_lock()
		{
#ifdef __linux__
			pthread_rwlock_rdlock(&lock);
#else
			AcquireSRWLockShared(&lock);
#endif
		}
		void r_unlock()
		{
#ifdef __linux__
			pthread_rwlock_unlock(&lock);
#else
			ReleaseSRWLockShared(&lock);
#endif
		}
		void w_lock()
		{
#ifdef __linux__
			pthread_rwlock_wrlock(&lock);
#else
			AcquireSRWLockExclusive(&lock);
#endif
		}
		void w_unlock()
		{
#ifdef __linux__
			pthread_rwlock_unlock(&lock);
#else
			ReleaseSRWLockExclusive(&lock);
#endif
		}
	};

	class rlock_guard
	{
	private:
		rwlock &rlock;
	public:
		rlock_guard(rwlock &lock) : rlock(lock)
		{
			rlock.r_lock();
		}
		~rlock_guard()
		{
			rlock.r_unlock();
		}
	};

	class wlock_guard
	{
	private:
		rwlock &wlock;
	public:
		wlock_guard(rwlock &lock) : wlock(lock)
		{
			wlock.w_lock();
		}
		~wlock_guard()
		{
			wlock.w_unlock();
		}
	};
}
