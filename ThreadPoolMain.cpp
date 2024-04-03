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
#include<chrono>
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
		std::future<R> push(T&& work,args&&... value) {
			if (shutdown) {
				throw std::exception("already shutdown");
			}
			//std::shared_ptr<std::promise<R>> pro = std::make_shared << std::promise<R >> ();

			std::shared_ptr<std::packaged_task<R()>> task = std::make_shared<std::packaged_task<R()>>(
				//rvalue도 함수 안에서는 다시 lvalue가 되므로 참조로 캡쳐 후, 함수 내부에서 rvalue로 바꿔준다.
				[w = std::forward<T>(work), &value...] () mutable {w(std::forward<args...>(value...)); }
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
	class testclass {
	public:
		int value;
		testclass() :value(1) {
			std::cout << "default" << std::endl;
		}
		testclass(const testclass& value) : testclass() {
			std::cout << "cons" << std::endl;
		}
		testclass(const testclass&& value) noexcept :testclass()  {
			std::cout << "rvalue cons" << std::endl;
		}
		testclass& operator=(testclass& value) {
			std::cout << "= lvalue" << std::endl;
			return *this;
		}
		testclass& operator=(testclass&& value) noexcept {
			std::cout << "= rvalue" << std::endl;
			return *this;
		}
	};
	void ThreadMain() {
		
		ThreadPool tp;
		testclass a,b;
		//void는 되나 다른 반환형이 있는 경우 안된다.
		std::future<testclass> re = tp.push([](testclass&& value) ->testclass
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << value.value << std::endl;
				return value;
			}, testclass());
		re.get();
		//std::cout << re.get().value << std::endl;

	}
#endif // _DEBUG
}