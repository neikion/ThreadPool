#include "ThreadPoolMain.h"
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <future>

namespace ThreadPool {

	class ThreadPool {
	public:
		std::mutex AccessJob;
		std::queue<std::function<void()>> works;
		size_t limit;
		std::vector<std::thread> workers;
		std::condition_variable AccessNoti;
		bool shutdown;
		~ThreadPool() {
			shutdown = true;
			AccessNoti.notify_all();
			for (size_t i = 0; i < workers.size(); i++) {
				workers[i].join();
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
		void readyWorks() {
			workers.reserve(limit);
			for (size_t i = 0; i < workers.size(); i++) {
				workers.push_back(std::thread(&ThreadPool::worker,this));
			}
		}
		ThreadPool() : shutdown(false), limit(std::thread::hardware_concurrency()) {
			readyWorks();
		}
		ThreadPool(size_t size) : limit(size), shutdown(false) {
			readyWorks();
		}
		//extern "C"�� ����ϱ� ���ؼ���
		//push_void_intArg1(void (*work)(), int arg1);
		//�� ���� ��Ȯ�� Ÿ���� ���������Ѵ�.(C������ �Լ� �����ε尡 �ȵȴ�.)
		template<typename T,typename... args>
		std::future<std::invoke_result_t<T, args...>> push(T work,args... value) {
			
		}


	};

	void ThreadMain() {

	}
}