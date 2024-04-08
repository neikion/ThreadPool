#pragma once
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
namespace ThreadPoolSpace {
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
		//ThreadPool is not copyable and moveable
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;

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

		template<typename T, typename... args, typename R = std::invoke_result_t<T, args...>>
		std::future<R> push(T&& work, args&&... value) {
			if (shutdown) {
				throw std::exception("already shutdown");
			}
			std::shared_ptr<std::promise<R>> pro = std::make_shared<std::promise<R>>();
			std::future<R> result = pro->get_future();
			{
				std::lock_guard<std::mutex> lg(AccessJob);
				works.push(
					//rvalue도 함수 안에서는 다시 lvalue가 되므로 참조로 캡쳐 후, 함수 내부에서 rvalue로 바꿔준다.
					[pro, w = std::forward<T>(work), &value...]() mutable
					{
						try {
							if constexpr (std::is_void_v<R>) {
								w(std::forward<args>(value)...);
								pro->set_value();
							}
							else {
								pro->set_value(w(std::forward<args>(value)...));
							}
						}
						catch (...) {
							pro->set_exception(std::current_exception());
						}
					}
				);
			}
			AccessNoti.notify_one();
			return result;
		}
	};
}