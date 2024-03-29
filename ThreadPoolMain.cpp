#include "ThreadPoolMain.h"
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>



namespace ThreadPool {
	extern "C" {
		class ThreadPool {
		public:
			std::mutex AccessJob;
			std::queue<std::function<void()>> works;
			size_t limit;
			std::vector<std::thread> workers;
			std::condition_variable AccessNoti;
			bool shutdown;
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
		};
	}
}