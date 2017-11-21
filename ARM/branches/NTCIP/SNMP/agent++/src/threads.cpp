/*_############################################################################
  _## 
  _##  threads.cpp  
  _## 
  _##
  _##  AGENT++ API Version 3.5.31
  _##  -----------------------------------------------
  _##  Copyright (C) 2000-2010 Frank Fock, Jochen Katz
  _##  
  _##  LICENSE AGREEMENT
  _##
  _##  WHEREAS,  Frank  Fock  and  Jochen  Katz  are  the  owners of valuable
  _##  intellectual  property rights relating to  the AGENT++ API and wish to
  _##  license AGENT++ subject to the  terms and conditions set forth  below;
  _##  and
  _##
  _##  WHEREAS, you ("Licensee") acknowledge  that Frank Fock and Jochen Katz
  _##  have the right  to grant licenses  to the intellectual property rights
  _##  relating to  AGENT++, and that you desire  to obtain a license  to use
  _##  AGENT++ subject to the terms and conditions set forth below;
  _##
  _##  Frank  Fock    and Jochen   Katz   grants  Licensee  a  non-exclusive,
  _##  non-transferable, royalty-free  license  to use   AGENT++ and  related
  _##  materials without  charge provided the Licensee  adheres to all of the
  _##  terms and conditions of this Agreement.
  _##
  _##  By downloading, using, or  copying  AGENT++  or any  portion  thereof,
  _##  Licensee  agrees to abide  by  the intellectual property  laws and all
  _##  other   applicable laws  of  Germany,  and  to all of   the  terms and
  _##  conditions  of this Agreement, and agrees  to take all necessary steps
  _##  to  ensure that the  terms and  conditions of  this Agreement are  not
  _##  violated  by any person  or entity under the  Licensee's control or in
  _##  the Licensee's service.
  _##
  _##  Licensee shall maintain  the  copyright and trademark  notices  on the
  _##  materials  within or otherwise  related   to AGENT++, and  not  alter,
  _##  erase, deface or overprint any such notice.
  _##
  _##  Except  as specifically   provided in  this  Agreement,   Licensee  is
  _##  expressly   prohibited  from  copying,   merging,  selling,   leasing,
  _##  assigning,  or  transferring  in  any manner,  AGENT++ or  any portion
  _##  thereof.
  _##
  _##  Licensee may copy materials   within or otherwise related   to AGENT++
  _##  that bear the author's copyright only  as required for backup purposes
  _##  or for use solely by the Licensee.
  _##
  _##  Licensee may  not distribute  in any  form  of electronic  or  printed
  _##  communication the  materials  within or  otherwise  related to AGENT++
  _##  that  bear the author's  copyright, including  but  not limited to the
  _##  source   code, documentation,  help  files, examples,  and benchmarks,
  _##  without prior written consent from the authors.  Send any requests for
  _##  limited distribution rights to fock@agentpp.com.
  _##
  _##  Licensee  hereby  grants  a  royalty-free  license  to  any  and   all 
  _##  derivatives  based  upon this software  code base,  that  may  be used
  _##  as a SNMP  agent development  environment or a  SNMP agent development 
  _##  tool.
  _##
  _##  Licensee may  modify  the sources  of AGENT++ for  the Licensee's  own
  _##  purposes.  Thus, Licensee  may  not  distribute  modified  sources  of
  _##  AGENT++ without prior written consent from the authors. 
  _##
  _##  The Licensee may distribute  binaries derived from or contained within
  _##  AGENT++ provided that:
  _##
  _##  1) The Binaries are  not integrated,  bundled,  combined, or otherwise
  _##     associated with a SNMP agent development environment or  SNMP agent
  _##     development tool; and
  _##
  _##  2) The Binaries are not a documented part of any distribution material. 
  _##
  _##
  _##  THIS  SOFTWARE  IS  PROVIDED ``AS  IS''  AND  ANY  EXPRESS OR  IMPLIED
  _##  WARRANTIES, INCLUDING, BUT NOT LIMITED  TO, THE IMPLIED WARRANTIES  OF
  _##  MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE  ARE DISCLAIMED.
  _##  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  _##  INDIRECT,   INCIDENTAL,  SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
  _##  (INCLUDING,  BUT NOT LIMITED  TO,  PROCUREMENT OF SUBSTITUTE  GOODS OR
  _##  SERVICES; LOSS OF  USE,  DATA, OR PROFITS; OR  BUSINESS  INTERRUPTION)
  _##  HOWEVER CAUSED  AND ON ANY THEORY  OF  LIABILITY, WHETHER IN CONTRACT,
  _##  STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  _##  IN  ANY WAY OUT OF  THE USE OF THIS  SOFTWARE,  EVEN IF ADVISED OF THE
  _##  POSSIBILITY OF SUCH DAMAGE. 
  _##
  _##
  _##  Stuttgart, Germany, Thu Sep  2 00:07:56 CEST 2010 
  _##  
  _##########################################################################*/


#include <errno.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#ifndef _POSIX_TIMERS
#include <sys/select.h>
#endif
#endif
#include <agent_pp/threads.h>
#include <agent_pp/mib_entry.h>
#include <agent_pp/mib.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif

#ifdef _THREADS
Synchronized ThreadManager::global_lock;
#endif

/**
 * Default constructor
 */
ThreadManager::ThreadManager()
{
}

/**
 * Destructor
 */
ThreadManager::~ThreadManager()
{
#ifdef _THREADS
    if (trylock())
	unlock();
    else
	unlock();
#endif    
}

/**
 * Start synchronized execution.
 */
void ThreadManager::start_synch()
{
#ifdef _THREADS
	lock();
#endif
}

/**
 * End synchronized execution.
 */
void ThreadManager::end_synch()
{
#ifdef _THREADS
	unlock();
#endif
}

/**
 * Start global synchronized execution.
 */
void ThreadManager::start_global_synch()
{
#ifdef _THREADS
	global_lock.lock();
#endif
}

/**
 * End global synchronized execution.
 */
void ThreadManager::end_global_synch()
{
#ifdef _THREADS
	global_lock.unlock();
#endif
}


ThreadSynchronize::ThreadSynchronize(ThreadManager& sync): s(sync)
{
#ifdef _THREADS
	s.start_synch();
#endif
}

ThreadSynchronize::~ThreadSynchronize()
{
#ifdef _THREADS
	s.end_synch();
#endif
}

SingleThreadObject::SingleThreadObject(): ThreadManager()
{
	start_synch();
}

SingleThreadObject::~SingleThreadObject()
{
	end_synch();
}

#ifdef _THREADS

/*--------------------- class Synchronized -------------------------*/

Synchronized::Synchronized()
{
#ifdef POSIX_THREADS
	int result;

	memset(&monitor, 0, sizeof(monitor));
	result = pthread_mutex_init(&monitor, 0);
	if (result) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("Synchronized mutex_init failed with (result)");
		LOG(result);
		LOG_END;
	}

	memset(&cond, 0, sizeof(cond));
	result = pthread_cond_init(&cond, 0);
	if (result) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("Synchronized cond_init failed with (result)");
		LOG(result);
		LOG_END;
	}
#else
#ifdef WIN32
	// Semaphore initially auto signaled, auto reset mode, unnamed
	semEvent = CreateEvent(0, FALSE, FALSE, 0);
	// Semaphore initially unowned, unnamed
	semMutex = CreateMutex(0, FALSE, 0);
	isLocked = FALSE;
#endif
#endif
}


Synchronized::~Synchronized()
{
#ifdef POSIX_THREADS
	int result;

	result = pthread_cond_destroy(&cond);
	if (result) {
		LOG_BEGIN(ERROR_LOG | 2);
		LOG("Synchronized cond_destroy failed with (result)(ptr)");
		LOG(result);
		LOG((unsigned long)this);
		LOG_END;
	}
	result = pthread_mutex_destroy(&monitor);
	if (result) {
		LOG_BEGIN(ERROR_LOG | 2);
		LOG("Synchronized mutex_destroy failed with (result)(ptr)");
		LOG(result);
		LOG((unsigned long)this);
		LOG_END;
	}
#else
#ifdef WIN32
	CloseHandle(semEvent);
	CloseHandle(semMutex);
#endif
#endif
}


void Synchronized::wait() {
#ifdef POSIX_THREADS
	cond_timed_wait(0);
#else
#ifdef WIN32
	wait(INFINITE);
#endif
#endif  
}

#ifdef POSIX_THREADS
int Synchronized::cond_timed_wait(const struct timespec *ts) 
{
  int result;
  if (ts) 
	result = pthread_cond_timedwait(&cond, &monitor, ts);
  else 
	result = pthread_cond_wait(&cond, &monitor);
  return result;
}
#endif

int Synchronized::wait(unsigned long timeout)
{
	boolean timeoutOccurred = FALSE;
#ifdef POSIX_THREADS
	struct timespec ts;
	struct timeval  tv;
	gettimeofday(&tv, 0);
	ts.tv_sec  = tv.tv_sec  + (int)timeout/1000;
	ts.tv_nsec = (tv.tv_usec + (timeout %1000)*1000) * 1000; 

	int err;
	if ((err = cond_timed_wait(&ts)) > 0) {
		switch(err) {
		case ETIMEDOUT:
		  timeoutOccurred = TRUE;
		  break;
		default:
		  LOG_BEGIN(ERROR_LOG | 1);
		  LOG("Synchronized: wait with timeout returned (error)");
		  LOG(err);
		  LOG_END;
		  break;
		}
	}
#else
#ifdef WIN32
	isLocked = FALSE;
	if (!ReleaseMutex(semMutex)) {
		LOG_BEGIN(ERROR_LOG | 2);
		LOG("Synchronized: releasing mutex failed");
		LOG_END;
	}
	int err;
	err = WaitForSingleObject(semEvent, timeout);
	switch (err) {
	case WAIT_TIMEOUT:
		LOG_BEGIN(EVENT_LOG | 8);
		LOG("Synchronized: timeout on wait");
		LOG_END;
		timeoutOccurred = TRUE;
		break;
	case WAIT_ABANDONED:
		LOG_BEGIN(ERROR_LOG | 2);
		LOG("Synchronized: waiting for event failed");
		LOG_END;
	}
	if (WaitForSingleObject (semMutex, INFINITE) != WAIT_OBJECT_0) {
		LOG_BEGIN(WARNING_LOG | 8);
		LOG("Synchronized: waiting for mutex failed");
		LOG_END;
	}
	isLocked = TRUE;
#endif 
#endif
	return timeoutOccurred;
}

void Synchronized::notify() {
#ifdef POSIX_THREADS
	int result;
	result = pthread_cond_signal(&cond);
	if (result) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("Synchronized: notify failed (result)");
		LOG(result);
		LOG_END;
	}
#else
#ifdef WIN32
	numNotifies = 1;
	if (!SetEvent(semEvent)) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("Synchronized: notify failed");
		LOG_END;
	}
#endif
#endif
}


void Synchronized::notify_all() {
#ifdef POSIX_THREADS
	int result;
	result = pthread_cond_broadcast(&cond);
	if (result) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("Synchronized: notify_all failed (result)");
		LOG(result);
		LOG_END;
	}
#else
#ifdef WIN32
	numNotifies = (char)0x80;
	while (numNotifies--)
		if (!SetEvent(semEvent)) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("Synchronized: notify failed");
			LOG_END;
		}
#endif 
#endif 
}

void Synchronized::lock() {
#ifdef POSIX_THREADS
    pthread_mutex_lock(&monitor);
#else
#ifdef WIN32
    if (WaitForSingleObject(semMutex, INFINITE) != WAIT_OBJECT_0) {
	LOG_BEGIN(ERROR_LOG | 1);
	LOG("Synchronized: lock failed");
	LOG_END;
	return;
    }
    if (isLocked) {
	// This thread owns already the lock, but
        // we do not like recursive locking. Thus
        // release it immediately and print a warning!
	if (!ReleaseMutex(semMutex)) {
		LOG_BEGIN(WARNING_LOG | 1);
		LOG("Synchronized: unlock failed");
		LOG_END;
	}      
	LOG_BEGIN(WARNING_LOG | 0);
	LOG("Synchronized: recursive locking detected!");
	LOG_END;	
    }
    isLocked = TRUE;
#endif
#endif
}

void Synchronized::unlock() {
#ifdef POSIX_THREADS
	pthread_mutex_unlock(&monitor);
#else
#ifdef WIN32
	isLocked = FALSE;
	if (!ReleaseMutex(semMutex)) {
		LOG_BEGIN(WARNING_LOG | 1);
		LOG("Synchronized: unlock failed");
		LOG_END;
		return;
	}
#endif
#endif
}

boolean Synchronized::trylock() {
#ifdef POSIX_THREADS
	if (pthread_mutex_trylock(&monitor) == 0)
		return TRUE;
	else
		return FALSE;
#else
#ifdef WIN32
	int status = WaitForSingleObject(semMutex, 0);
	if (status != WAIT_OBJECT_0) {
		LOG_BEGIN(DEBUG_LOG | 9);
		LOG("Synchronized: try lock failed");
		LOG_END;
		return FALSE;
	}
	if (isLocked) {
		if (!ReleaseMutex(semMutex)) {
			LOG_BEGIN(WARNING_LOG | 1);
			LOG("Synchronized: unlock failed");
			LOG_END;
		}
		return FALSE;
	}
	else
		isLocked = TRUE;
	return TRUE;
#endif
#endif
}


/*------------------------ class Thread ----------------------------*/

ThreadList Thread::threadList; 

#ifdef POSIX_THREADS
void* thread_starter(void* t)
{
	Thread *thread = (Thread*)t;
	Thread::threadList.add(thread);

#ifndef NO_FAST_MUTEXES
	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("Thread: started (tid)");
	LOG((int)(thread->tid));
	LOG_END;
#endif

	thread->get_runnable().run();

#ifndef NO_FAST_MUTEXES
	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("Thread: ended (tid)");
	LOG((int)(thread->tid));
	LOG_END;
#endif
	Thread::threadList.remove(thread);
	thread->status = Thread::FINISHED;

	return t;
}
#else
#ifdef WIN32
DWORD thread_starter(LPDWORD lpdwParam)
{
	Thread *thread = (Thread*) lpdwParam;
	Thread::threadList.add(thread);

	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("Thread: started (tid)");
	LOG(thread->tid);
	LOG_END;

	thread->get_runnable().run();

	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("Thread: ended (tid)");
	LOG(thread->tid);
	LOG_END;

	Thread::threadList.remove(thread);
	thread->status = Thread::FINISHED;

	::SetEvent(thread->threadEndEvent);

	return 0;	
}
#endif
#endif

Thread::Thread()
{
	stackSize = AGENTPP_DEFAULT_STACKSIZE;
	runnable  = (Runnable*)this;
	status    = IDLE;
#ifdef WIN32
	threadHandle = INVALID_HANDLE_VALUE;
	threadEndEvent = ::CreateEvent(NULL, true, false, NULL);
#endif //WIN32
}

Thread::Thread(Runnable &r)
{
	stackSize = AGENTPP_DEFAULT_STACKSIZE;
	runnable  = &r;
	status    = IDLE;
#ifdef WIN32
	threadHandle = INVALID_HANDLE_VALUE;
	threadEndEvent = ::CreateEvent(NULL, true, false, NULL);
#endif //WIN32
}

void Thread::run()
{
	LOG_BEGIN(ERROR_LOG | 1);
	LOG("Thread: empty run method!");
	LOG_END;
}

Thread::~Thread() 
{ 
	if (status != IDLE)
	{
		join(); 
	}
#ifdef WIN32
	::CloseHandle(threadEndEvent);
	if(threadHandle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(threadHandle);
		threadHandle = INVALID_HANDLE_VALUE;
	}
#endif
}


Runnable& Thread::get_runnable()
{
	return *runnable;
}

void Thread::join()
{
#ifdef POSIX_THREADS
	if (status) {
		void* retstat;
		int err = pthread_join(tid, &retstat);
		if (err) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("Thread: join failed (error)");
			LOG(err);
			LOG_END;
		}
		status = IDLE;
		LOG_BEGIN(DEBUG_LOG | 4);
		LOG("Thread: joined thread successfully (tid)");
		LOG((int)tid);
		LOG_END;
	}
	else {
		LOG_BEGIN(WARNING_LOG | 1);
		LOG("Thread: thread not running (tid)");
		LOG((int)tid);
		LOG_END;
	}
#else
#ifdef WIN32
	if (status) {
		if (WaitForSingleObject(threadEndEvent, 
					INFINITE) != WAIT_OBJECT_0) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("Thread: join failed");
			LOG_END;
		}
		status = IDLE;
		LOG_BEGIN(DEBUG_LOG | 4);
		LOG("Thread: joined thread successfully");
		LOG_END;
	}
	else {
		LOG_BEGIN(WARNING_LOG | 1);
		LOG("Thread: thread not running");
		LOG_END;
	}
#endif
#endif
}

void Thread::start()
{
#ifdef POSIX_THREADS
	if (status == IDLE) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, stackSize);
		int err = pthread_create(&tid, &attr, thread_starter, this);
		if (err) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("Thread: cannot start thread (error)");
			LOG(err);
			LOG_END;
			status = IDLE;
		}
		else 
			status = RUNNING;
		pthread_attr_destroy(&attr);
	}
	else {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("Thread: thread already running!");
		LOG_END;
	}
#else
#ifdef WIN32
	DWORD *targ = (DWORD*)this;
  
	if (status == IDLE) {

        if(threadHandle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(threadHandle);
			threadHandle = INVALID_HANDLE_VALUE;
		}

        threadHandle = 
		  CreateThread (0, // no security attributes
				stackSize, 
				(LPTHREAD_START_ROUTINE)thread_starter, 
				targ, 
				0,   
				&tid);   
		
		if (threadHandle == 0) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("Thread: cannot start thread");
			LOG_END;
			status = IDLE;
		}
		else {
			status = RUNNING;
		}
	}
	else {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("Thread: thread already running!");
		LOG_END;
	}
#endif
#endif
}

void  Thread::sleep(long millis)
{
#ifdef WIN32
	Sleep (millis);
#else
	nsleep((int)(millis/1000), (millis%1000)*1000000);
#endif
}

void Thread::sleep(long millis, int nanos)
{
#ifdef WIN32
	sleep (millis);
#else
	nsleep((int)(millis/1000), (millis%1000)*1000000 + nanos);
#endif
}

void Thread::nsleep(int secs, long nanos)
{
#ifdef WIN32
	DWORD millis = secs*1000 + nanos/1000000;
	Sleep(millis);
#else
	long s = secs + nanos / 1000000000;
	long n = nanos % 1000000000;

#ifdef _POSIX_TIMERS
	struct timespec interval, remainder;
	interval.tv_sec = (int)s;
	interval.tv_nsec = n;
	if (nanosleep(&interval, &remainder) == -1) {
		if (errno == EINTR) {
			LOG_BEGIN(EVENT_LOG | 3);
			LOG("Thread: sleep interrupted");
			LOG_END;
		}
	}
#else
	struct timeval interval;
	interval.tv_sec = s;
	interval.tv_usec = n/1000;
	fd_set writefds, readfds, exceptfds;
	FD_ZERO(&writefds);
	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	if (select(0, &writefds, &readfds, &exceptfds, &interval) == -1) {
		if (errno == EINTR) {
			LOG_BEGIN(EVENT_LOG | 3);
			LOG("Thread: sleep interrupted");
			LOG_END;
		}
	}
#endif
#endif
}

#ifdef AGENTPP_USE_THREAD_POOL

/*--------------------- class TaskManager --------------------------*/

TaskManager::TaskManager(ThreadPool *tp, int stackSize):thread(*this)
{
	threadPool = tp;
	task       = 0;
	go         = TRUE;
	thread.set_stack_size(stackSize);
	thread.start();
	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("TaskManager: thread started");
	LOG_END;
}

TaskManager::~TaskManager() {
	lock();
	go = FALSE;
	notify();
	unlock();
	thread.join();
	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("TaskManager: thread stopped");
	LOG_END;
}

void TaskManager::run() {
	lock();
	while (go) {
		if (task) {
			task->run();
			delete task;
			task = 0;
			unlock();
			threadPool->idle_notification();
			lock();
		}
		else {
			wait();
		}
	}
	unlock();
}

boolean TaskManager::set_task(Runnable *t)
{
	lock();
	if (!task) {
		task = t;
		notify();
		unlock();
		LOG_BEGIN(DEBUG_LOG | 2);
		LOG("TaskManager: after notify");
		LOG_END;
		return TRUE;
	}
	else {
		unlock();
		LOG_BEGIN(DEBUG_LOG | 2);
		LOG("TaskManager: got already a task");
		LOG_END;
		return FALSE;
	}
}

/*--------------------- class ThreadPool --------------------------*/


void ThreadPool::execute(Runnable *t)
{
	lock();
	TaskManager *tm = 0;
	while (!tm) {
		ArrayCursor<TaskManager> cur;
		for (cur.init(&taskList); cur.get(); cur.next()) {
			tm = cur.get();
			if (tm->is_idle()) {
				LOG_BEGIN(DEBUG_LOG | 1);
				LOG("TaskManager: task manager found");
				LOG_END;

				unlock();
				if (tm->set_task(t)) {
				    return;
				}
				else {
				    // task could not be assigned
				    tm = 0;
				    lock();
				}
			}
			tm = 0;
		}
		if (!tm) wait(1000);
	}
	unlock();
}

boolean ThreadPool::is_idle() 
{
	lock();
	ArrayCursor<TaskManager> cur;
	for (cur.init(&taskList); cur.get(); cur.next()) {
		if (!cur.get()->is_idle()) {
			unlock();
			return FALSE;
		}
	}
	unlock();
	return TRUE;
}

ThreadPool::ThreadPool(int size)
{
	for (int i=0; i<size; i++) {
		taskList.add(new TaskManager(this));
	}
}

ThreadPool::ThreadPool(int size, int stack_size)
{
	stackSize = stack_size;
	for (int i=0; i<size; i++) {
		taskList.add(new TaskManager(this, stackSize));
	}
} 

ThreadPool::~ThreadPool()
{
}

/*--------------------- class QueuedThreadPool --------------------------*/

QueuedThreadPool::QueuedThreadPool(int size): ThreadPool(size)
{
}

QueuedThreadPool::QueuedThreadPool(int size, int stack_size):
	ThreadPool(size, stack_size)
{
} 

QueuedThreadPool::~QueuedThreadPool()
{
	go = FALSE;
	join();
}

void QueuedThreadPool::assign(Runnable* t) 
{
	TaskManager *tm = 0;
	ArrayCursor<TaskManager> cur;
	for (cur.init(&taskList); cur.get(); cur.next()) {
		tm = cur.get();
		if (tm->is_idle()) {				
			LOG_BEGIN(DEBUG_LOG | 1);
			LOG("TaskManager: task manager found");
			LOG_END;
			Thread::unlock();
			if (!tm->set_task(t)) {
			    tm = 0;
			    Thread::lock();
			}
			else {
			    Thread::lock();
			    break;
			}
		}
		tm = 0;
	}
	if (!tm) {
		queue.add(t);
		Thread::notify();
	}
}

void QueuedThreadPool::execute(Runnable *t)
{
	Thread::lock();
	assign(t);
	Thread::unlock();
}

void QueuedThreadPool::run() 
{
	go = TRUE;
	Thread::lock();
	while (go) {
		Runnable* t = queue.removeFirst();
		if (t) {
			assign(t);
		}
		Thread::wait(1000);
	}
	Thread::unlock();
}

void QueuedThreadPool::idle_notification() 
{
	Thread::lock();
	Thread::notify();
	Thread::unlock();
	ThreadPool::idle_notification();
}


void MibTask::run()
{
	(task->called_class->*task->method)(task->req);       	
}

#ifdef NO_FAST_MUTEXES

LockRequest::LockRequest(Synchronized* s)
{ 
	target = s; 
	lock();
}

LockRequest::~LockRequest()
{
	unlock();
}
 
LockQueue::LockQueue()
{
	go = TRUE;
	start();
}

LockQueue::~LockQueue()
{
	go = FALSE;
	lock();
	// wake up queue
	notify();
	unlock();

	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("LockQueue: end queue");
	LOG_END;

	// join thread here, before pending list is deleted 
	if (is_alive()) join();

	LOG_BEGIN(DEBUG_LOG | 1);
	LOG("LockQueue: queue stopped");
	LOG_END;

	pendingRelease.clear();
	pendingLock.clear();
}

void LockQueue::run() 
{
	lock();
	while ((!pendingLock.empty()) || (!pendingRelease.empty()) || (go)) {
		while (!pendingRelease.empty()) {
			LockRequest* r = pendingRelease.removeFirst();
			r->target->unlock();
			r->lock();
			r->notify();
			r->unlock();
		}
		int pl = pendingLock.size();
		int pending = pl;
		for (int i=0; i<pl; i++) {
			LockRequest* r = pendingLock.removeFirst();
			if (r->target->trylock()) {
				r->lock();
				r->notify();
				r->unlock();
				pending--;
			}
			else 
				pendingLock.addLast(r);
		}
		LOG_BEGIN(DEBUG_LOG | 8);
		LOG("LockQueue: waiting for next event (pending)");
		LOG(pending);
		LOG_END;

		// do not wait forever because we cannot 
		// be sure that all instrumentation code notifies
		// us correctly.
		wait(5000);
	}
	unlock();
}

void LockQueue::acquire(LockRequest* r)
{
	lock();
	LOG_BEGIN(DEBUG_LOG | 2);
	LOG("LockQueue: adding lock request (ptr)");
	LOG((int)r->target);
	LOG_END;
	pendingLock.addLast(r);
	notify();
	unlock();
}

void LockQueue::release(LockRequest* r)
{
	lock();
	LOG_BEGIN(DEBUG_LOG | 2);
	LOG("LockQueue: adding release request (ptr)");
	LOG((int)r->target);
	LOG_END;
	pendingRelease.addLast(r);
	notify();
	unlock();
}


#endif // NO_FAST_MUTEXES

#endif 
#endif // _THREADS

#ifdef AGENTPP_NAMESPACE
}
#endif








