#include "AudioThread.h"

#include "Logger.h"

#include <thread>
#include <chrono>

namespace ProdCast {

	using namespace std::chrono_literals;

	ThreadPool::ThreadPool() {
		m_queueLength = 0;
		m_isRunning = false;
		m_nbThreads = 0;
		m_threads = nullptr;
	}
	
	void ThreadPool::InitPool(int nbThreads) {
		m_nbThreads = nbThreads;
		m_isRunning = true;
		if (m_nbThreads > 0) {

			m_threads = new WorkerThread[m_nbThreads];
			for (int i = 0; i < m_nbThreads; i++) {
				m_threads[i] = WorkerThread();
				m_threads[i].Init(this);
			}
		}
		PC_TRACE("Thread Pool initialized");
	}

	void ThreadPool::DeInitPool() {
		std::scoped_lock lock{ m_poolMutex };
		m_isRunning = false;
		m_cv.notify_all();
		lock.~scoped_lock();
		for (int i = 0; i < m_nbThreads; i++) {
			m_threads[i].Stop();
		}
		PC_TRACE("Thread Pool deinitialized");
	}

	ThreadPool::~ThreadPool() {
		delete[] m_threads; // ?
	}

	void ThreadPool::addJob(ThreadableJob* job) {
		std::scoped_lock lock{ m_poolMutex };
		if (m_nbThreads == 0) {
			job->Process();
			return;
		}
		m_jobs.push(job);
		m_queueLength++;
		m_cv.notify_one();
	}

	void ThreadPool::getNextJob(ThreadableJob*& job) {
		std::unique_lock lock{ m_poolMutex };
		while(m_jobs.empty()) {
			if (!m_isRunning) 
				return;
			m_cv.wait(lock);
		}

		job = m_jobs.front();
		m_jobs.pop();
		m_queueLength--;
	}

	void WorkerThread::Init(ThreadPool* parent) {
		m_parent = parent;
		m_thread = std::thread(&WorkerThread::Work, this);
		PC_TRACE("Started worker thread");
	}

	void WorkerThread::Work() {
		while (true) {
			ThreadableJob* job{};
			m_parent->getNextJob(job);
			if (job)
				job->Process();
			else
				return;
		}
	}

	void WorkerThread::Stop() {
		m_thread.join();
		PC_TRACE("Stopped worker thread");
	}
}