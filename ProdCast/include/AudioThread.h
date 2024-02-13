#pragma once

#include <shared_mutex>
#include <queue>
#include <condition_variable>

namespace ProdCast {

	class WorkerThread;
	
	class ThreadableJob {

	public:
		virtual void Process() = 0;
	};

	class ThreadPool {
	public:
		ThreadPool();
		~ThreadPool();
		void InitPool(int nbThreads);
		void DeInitPool();

		void addJob(ThreadableJob* job);
		void getNextJob(ThreadableJob*& job);

		unsigned int m_queueLength;
		int m_nbThreads;
		std::queue<ThreadableJob*> m_jobs;
		WorkerThread* m_threads;
		std::shared_mutex m_poolMutex;
		std::condition_variable_any m_cv;
		bool m_isRunning;
	};

	class WorkerThread {
	public:
		WorkerThread() = default;
		void Init(ThreadPool* parent);
		void Work();
		void Stop();

	private:
		ThreadPool* m_parent;
		std::thread m_thread;
	};
}

