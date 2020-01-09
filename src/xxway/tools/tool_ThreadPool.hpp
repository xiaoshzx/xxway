#ifndef __TOOL_THREADPOOL_HPP__
#define __TOOL_THREADPOOL_HPP__

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>

// 线程池类
class tool_ThreadPool {
public:
	tool_ThreadPool (size_t _threads) {
		for (size_t i = 0; i < _threads; ++i) {
			m_workers.emplace_back ([this] {
				while (true) {
					std::unique_lock<std::mutex> ul (m_mutex);
					m_condition.wait (ul, [this] { return !m_tasks.empty (); });
					std::function<void ()> _task = std::move (m_tasks.front ());
					m_tasks.pop ();
					ul.unlock ();
					_task ();
				}
			});
		}
	}

	template<class F, class... Args>
	std::future<typename std::result_of<F (Args...)>::type> enqueue (F &&f, Args &&... args) {
		using return_type = typename std::result_of<F (Args...)>::type;
		auto _task = std::make_shared<std::packaged_task<return_type ()>> (std::bind (std::forward<F> (f), std::forward<Args> (args)...));
		std::future<return_type> _res = _task->get_future ();
		std::unique_lock<std::mutex> ul (m_mutex);
		m_tasks.emplace ([_task] () { (*_task)(); });
		ul.unlock ();
		m_condition.notify_one ();
		return _res;
	}

	~tool_ThreadPool () {
		m_condition.notify_all ();
		for (std::thread &_worker : m_workers)
			_worker.join ();
	}

private:
	std::vector<std::thread>			m_workers;
	std::queue<std::function<void ()>>	m_tasks;
	std::mutex							m_mutex;
	std::condition_variable				m_condition;
};

#endif //__TOOL_THREADPOOL_HPP__
