#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <functional>
#include <future>
#include "CommonDefs.h"
#include "Event.h"

using namespace std;

typedef int ThreadId;

class ThreadPool
{
private:
	vector<thread> m_poolOfThreads;
	map<ThreadId, queue<function<void()>>> m_taskQueue;
	mutex* m_poolOfMutexs = nullptr;
	condition_variable* m_poolOfCondVars = nullptr;
	bool m_bStop = false;

public:
	ThreadPool(int totalThreads)
	{
		m_poolOfMutexs = new mutex[totalThreads];
		m_poolOfCondVars = new condition_variable[totalThreads];

		for (int idx = 0; idx < totalThreads; ++idx)
		{
			m_poolOfThreads.emplace_back(std::thread(&ThreadPool::workerThread, this, idx));
		}
	}

	~ThreadPool()
	{
		m_bStop = true;
		for (int idx = 0; idx < m_poolOfThreads.size(); ++idx)
		{
			if (m_poolOfThreads[idx].joinable())
				m_poolOfThreads[idx].join();
		}

		delete m_poolOfMutexs;
		delete m_poolOfCondVars;
	}

	void workerThread(const int threadIndex)
	{
		while (true)
		{
			unique_lock<mutex> lck(m_poolOfMutexs[threadIndex]);
			m_poolOfCondVars[threadIndex].wait(lck,
				[&]() { return !m_taskQueue[threadIndex].empty() || m_bStop; });

			if (m_bStop)
				break;

			auto task = std::move(m_taskQueue[threadIndex].front());
			m_taskQueue[threadIndex].pop();
			lck.unlock();
			task();
		}
	}

	template <class F, class... Args>
	void enqueuTask(int threadNumber, F&& func, Args&&... args)
	{
		function<void()> task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

		unique_lock<mutex> lck(m_poolOfMutexs[threadNumber]);
		m_taskQueue[threadNumber].emplace(task);
		lck.unlock();
		m_poolOfCondVars[threadNumber].notify_one();
	}

};


class KeyedExecutor
{
	ThreadPool* m_threadPool = nullptr;
	uint16_t m_totalThread = 0;
public:
	KeyedExecutor(uint16_t totalThread) : m_totalThread(totalThread)
	{
		m_threadPool = new ThreadPool(totalThread);
	}
	~KeyedExecutor()
	{
		if (m_threadPool)
			delete m_threadPool;

		m_threadPool = nullptr;
	}

	template<class F, class... Args>
	void submit(const string& id, F&& func, Args&&... args)
	{
		int threadIndex = std::hash<string>()(id.c_str()) % m_totalThread;
		thread t([&] { m_threadPool->enqueuTask(threadIndex, std::forward<F>(func), std::forward<Args>(args)...); });
		//t.detach();
		t.join();
	}

	template<class F, class... Args>
	Event get(const string& id, F&& func, Args&&... args)
	{
		int threadIndex = std::hash<string>()(id.c_str()) % m_totalThread;
	}
};