#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __linux__
#include <signal.h>
#include <sys/time.h>
#else //__WIN32__
#include <mmsystem.h>
#endif

class Timer
{
private:
#ifdef __linux__
	timer_t timer;
	int ret;
	static void sighandle(int signum, siginfo_t *info, void *arg)
	{
		Timer *t = (Timer *)(info->si_ptr);
		t->timeout();
	}
#else //__WIN32__
	MMRESULT timer_id;
	static void WINAPI onTimeFunc(UINT uTimerID,UINT uMsg,DWORD_PTR dwUser,DWORD_PTR dw1,DWORD_PTR dw2)
	{
		Timer *t = (Timer *)(dwUser);
		t->timeout();
	}
#endif
public:
	Timer()
	{
#ifdef __linux__
#else //__WIN32__
		timer_id = 0;
#endif
	}
	virtual ~Timer() { timerstop(); }

	virtual void timeout() {}

	bool timerstart(unsigned int msec)
	{
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
		ret = timer_create(CLOCK_MONOTONIC, &evp, &timer);
		if (ret != 0)
			return false;
		ts.it_value.tv_sec = msec / 1000;
		ts.it_value.tv_nsec = (msec % 1000) * 1000000;
		ts.it_interval = ts.it_value;
		return (0 == timer_settime(timer, 0, &ts, NULL));
#else //__WIN32__
		timer_id = timeSetEvent(msec, 1, (LPTIMECALLBACK)&onTimeFunc, DWORD(this), TIME_PERIODIC);
		return (0 != timer_id);
#endif
	}
	void timerstop()
	{
#ifdef __linux__
		if (ret == 0)
		{
			ret = -1;
			timer_delete(timer);
		}
#else //__WIN32__
		if (0 != timer_id)
		{
			timeKillEvent(timer_id);
			timer_id = 0;
		}
#endif
	}

protected:
};
	
#endif
