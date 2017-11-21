#pragma once

#ifdef __linux__
#include <signal.h>
#include <sys/time.h>
#else //__WIN32__
#include <windows.h>
//#include <mmsystem.h>
#endif
#include <functional>

namespace hik
{
	class timer
	{
	private:
		std::function<void()> timeout;
	#ifdef __linux__
		timer_t hTimer;
		int ret;
		static void sighandle(int signum, siginfo_t *info, void *arg)
		{
			timer *t = (timer *)(info->si_ptr);
			if (t->timeout)
				t->timeout();
			else
				t->Timeout();
		}
	#else //__WIN32__
		#if 0
		MMRESULT timer_id;
		static void WINAPI onTimeFunc(UINT uTimerID,UINT uMsg,DWORD_PTR dwUser,DWORD_PTR dw1,DWORD_PTR dw2)
		{
			timer *t = (timer *)(dwUser);
			if (t->timeout)
				t->timeout();
			else
				t->Timeout();
		}
		#else
		HANDLE hTimerQueue;
		HANDLE hTimer;
		static VOID CALLBACK TimerRoutine(PVOID lpParameter, BOOLEAN TimerOrWaitFired)  
		{  
		    timer *t = static_cast<timer *>(lpParameter);
			if (t->timeout)
				t->timeout();
			else
				t->Timeout();
		}
		#endif
	#endif
	public:
		timer(std::function<void()> f = nullptr) : timeout(f)
		{
	#ifdef __linux__
			ret = -1;
	#else //__WIN32__
		#if 0
			timer_id = 0;
		#else
			hTimerQueue = CreateTimerQueue();
			hTimer = nullptr;
		#endif
	#endif
		}
		
		virtual ~timer()
		{ 
			halt();
	#if defined(__WIN32__) || defined(WIN32) || defined(_MSC_VER)
			if (hTimerQueue != nullptr)
				DeleteTimerQueueEx(hTimerQueue, NULL);
	#endif
		}

		virtual void Timeout() {}

		bool boot(unsigned int msec, std::function<void()> f = nullptr)
		{
			if (msec == 0)
				return false;
			if (timeout == nullptr)
				timeout = f;
	#ifdef __linux__
			struct sigaction sigact;
			struct sigevent evp;
			struct itimerspec ts;
		
			sigemptyset(&sigact.sa_mask);
			sigact.sa_sigaction = sighandle;
			sigact.sa_flags = SA_SIGINFO;
			sigaction(SIGUSR2, &sigact, NULL);

			evp.sigev_value.sival_ptr = this;
			evp.sigev_notify = SIGEV_SIGNAL;
			evp.sigev_signo = SIGUSR2;
			ret = timer_create(CLOCK_MONOTONIC, &evp, &hTimer);
			if (ret != 0)
				return false;
			ts.it_value.tv_sec = msec / 1000;
			ts.it_value.tv_nsec = (msec % 1000) * 1000000;
			ts.it_interval = ts.it_value;
			return (0 == timer_settime(hTimer, 0, &ts, NULL));
	#else //__WIN32__
		#if 0	//此定时器虽精度较高，但定时时长无法超过1000s，因此废弃使用timerqueue定时器这也是msdn推荐的
			timer_id = timeSetEvent(msec, 1, (LPTIMECALLBACK)&onTimeFunc, DWORD(this), TIME_PERIODIC);
			return (0 != timer_id);
		#else
			if (hTimerQueue == nullptr)
				return false;
			return 0 != CreateTimerQueueTimer(&hTimer, hTimerQueue, WAITORTIMERCALLBACK(TimerRoutine), this, msec, msec, WT_EXECUTEDEFAULT);
		#endif
	#endif
		}
		void halt()
		{
	#ifdef __linux__
			if (ret == 0)
			{
				ret = -1;
				timer_delete(hTimer);
			}
	#else //__WIN32__
		#if 0
			if (0 != timer_id)
			{
				timeKillEvent(timer_id);
				timer_id = 0;
			}
		#else
			if (hTimerQueue != nullptr && hTimer != nullptr)
			{
				DeleteTimerQueueTimer(hTimerQueue, hTimer, NULL);
				hTimer = nullptr;
			}
		#endif
	#endif
		}

		bool reset(unsigned int msec)
		{
			halt();
			return boot(msec);
		}

	protected:
	};
}
