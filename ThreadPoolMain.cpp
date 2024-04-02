#include "ThreadPoolMain.h"
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#ifdef _DEBUG
#include<iostream>
#endif // _DEBUG
namespace ThreadPool {

	class ThreadPool {
	private:
		std::mutex AccessJob;
		std::queue<std::function<void()>> works;
		size_t limit;
		std::vector<std::thread> workers;
		std::condition_variable AccessNoti;
		bool shutdown;
		void readyWorks() {
			workers.reserve(limit);
			for (size_t i = 0; i < limit; i++) {
				workers.push_back(std::thread(&ThreadPool::worker, this));
			}
		}
		void worker() {
			while (!shutdown) {
				using namespace std;
				unique_lock<mutex> ul(AccessJob);
				AccessNoti.wait(ul, [self = this]() {return !self->works.empty() || self->shutdown; });
				if (shutdown) {
					return;
				}
				auto work = move(works.front());
				works.pop();
				ul.unlock();
				work();
			}
		}
	public:
		~ThreadPool() {
			shutdown = true;
			AccessNoti.notify_all();
			for (size_t i = 0; i < workers.size(); i++) {
				workers[i].join();
			}
		}
		ThreadPool() : ThreadPool(static_cast<size_t>(std::thread::hardware_concurrency())) {}
		ThreadPool(size_t size) : limit(size), shutdown(false) {
			readyWorks();
		}
		
		template<typename T,typename... args,typename R=std::invoke_result_t<T,args...>>
		std::future<R> push(T work,args... value) {
			if (shutdown) {
				throw std::exception("already shutdown");
			}
			std::shared_ptr<std::packaged_task<R()>> task = std::make_shared<std::packaged_task<R()>>(
				std::bind(std::forward<T>(work),std::forward<args>(value)...)
				
			);
			std::future<R> result = task->get_future();
			{
				std::lock_guard<std::mutex> lg(AccessJob);
				works.push(
					[task]
					{
						(*task)();
					}
				);
			}
			AccessNoti.notify_one();
			return result;
		}
	};
#ifdef _DEBUG
	void ThreadMain() {
		ThreadPool tp;
		std::future<void> re=tp.push([] ()->void {std::cout << "test" << std::endl; });
		re.get();

	}
#endif // _DEBUG
}