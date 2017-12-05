#pragma once 

#include <vector>
#include <functional>
#include <map>
#include "thread.h"
#include "mutex.h"
#include "sem.h"

namespace hik
{
	using Task = std::function<void()>;

	enum TaskPriority
	{
		TASK_FIRST = 0,
		TASK_SECOND = 1,
		TASK_THIRD = 2,
		TASK_FOURTH = 3,
		TASK_FIFTH = 4,
		TASK_SIXTH = 5,
		TASK_SEVENTH = 6,
		TASK_EIGHTH = 7,
	};

	class threadpool
	{
	private:
		sem semaphore;
		volatile bool running;

		struct processor : public hik::thread
		{
			threadpool &pool;
			processor(threadpool &p) : pool(p) {}
			void Run()
			{
				while (pool.running)
				{
					if (pool.semaphore.wait())
					{
						Task task = pool.gettask();
						if (task != nullptr)
							task();
					}
				}
			}			
		};
		std::vector<processor> threads;

		std::multimap<TaskPriority, Task> taskqueue;
		hik::mutex qmtx;

		Task gettask()
		{
			hik::mutex_guard guard(qmtx);
			if (taskqueue.empty())
				return nullptr;
			auto it = taskqueue.begin();
			Task task = it->second;
			taskqueue.erase(it);
			return task;
		}

	public:
		threadpool()
		{
			running = false;
		}

		~threadpool()
		{
			stop();
		}
		
		void start(int num)
		{
			if (!running)
			{
				running = true;
				for (int i = 0; i < num; i++)
					threads.emplace_back(*this);
				for (auto &th : threads)
					th.start();
			}
		}

		void stop()
		{
			if (running)
			{
				running = false;
				for (auto &th : threads)
					th.stop();
				threads.clear();
			}
		}

		void addtask(Task &&task, TaskPriority prior = TASK_EIGHTH)
		{
			addtask(task, prior);
		}

		void addtask(Task &task, TaskPriority prior = TASK_EIGHTH)
		{
			qmtx.lock();
			taskqueue.emplace(prior, task);
			qmtx.unlock();
			semaphore.post();
		}
	};
}
