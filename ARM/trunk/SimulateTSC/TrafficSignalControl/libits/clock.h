#ifndef __CLOCK_H__
#define __CLOCK_H__

#ifdef __linux__
#include <signal.h>
#include <sys/time.h>
#else //__WIN32__
#include <mmsystem.h>
#endif

#include "thread.h"
#include "protocol.h"
#include "ipc.h"

namespace HikClock
{
	using HikThread::Thread;
	using HikIts::Protocol;
	using HikIpc::Ipc;

    class Timer
    {
    private:
#ifdef __linux__
        struct sigevent evp;
        struct itimerspec ts;
        timer_t timer;
        int ret;
		static void sighandle(int signum, siginfo_t *info, void *arg)
        {
            Timer *timer = (Timer *)(info->si_ptr);
            timer->timeout();
        }
#else //__WIN32__
        MMRESULT timer_id;
        static void WINAPI onTimeFunc(UINT uTimerID,UINT uMsg,DWORD_PTR dwUser,DWORD_PTR dw1,DWORD_PTR dw2)
        {
            Timer *timer = (Timer *)(dwUser);
            timer->timeout();
        }
#endif
    public:
        Timer(void (Timer::*func)(void) = NULL)
        {
#ifdef __linux__
			struct sigaction sigact;
			sigemptyset(&sigact.sa_mask);
			sigact.sa_sigaction = sighandle;
			sigact.sa_flags = SA_SIGINFO;
            sigaction(SIGUSR2, &sigact, NULL);

            evp.sigev_value.sival_ptr = this;
            evp.sigev_notify = SIGEV_SIGNAL;
            evp.sigev_signo = SIGUSR2;
            ret = timer_create(CLOCK_MONOTONIC, &evp, &timer);
#else //__WIN32__
            timer_id = 0;
#endif
        }
        virtual ~Timer() { timerstop(); }

        virtual void timeout() {}

        bool timerstart(unsigned int msec)
        {
#ifdef __linux__
            if (ret != 0)
                return false;
            ts.it_value.tv_sec = 0;
            ts.it_value.tv_nsec = msec * 1000000;
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
	
    class Clock: public Thread, public Timer
	{
	private:
		Protocol & ptl;
		Ipc & ipc;
        int perSecTimes;	//每秒定时次数
        unsigned int perDelayMsec;	//每次定时毫秒数
        int times;
	public:
        Clock(Protocol & p, Ipc & i) : ptl(p), ipc(i)
        {
            perDelayMsec = 1000 / p.perSecTimes;	//每次定时毫秒数
            perSecTimes = p.perSecTimes;	//每秒定时次数
            times = 0;
            start();
        }
        ~Clock() {}
        void timeout()
        {
            ipc.SemPostTimerForPhaseDrv();
            if (++times == perSecTimes)
            {
                ipc.SemPostTimerForPhaseCtl();
                times = 0;
            }
        }
        void run(void *arg)
		{
			ipc.SemWaitStartTimer();
			ipc.SemPostTimerForPhaseCtl();
            timerstart(perDelayMsec);
		}
	protected:
	
	};
}

#endif
