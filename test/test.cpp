#include "../ThreadPoolMain.h"
#ifdef _DEBUG
#include<iostream>
#include<chrono>
#endif // _DEBUG
namespace ThreadPool {
#ifdef _DEBUG
	class testclass {
	public:
		int value;
		testclass() :value(1) {
			std::cout << "default" << std::endl;
		}
		testclass(const testclass& value) : value(2) {
			std::cout << "copy cons" << std::endl;
		}
		testclass(const testclass&& value) noexcept : value(3) {
			std::cout << "move cons" << std::endl;
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
		testclass a;

		std::future<void> result1 = tp.push([](testclass&& value) ->void
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << value.value << std::endl;
			}, std::move(a));
		result1.get();
		
		std::cout << std::endl;
		testclass b;
		std::future<void> result2 = tp.push([](testclass& value) ->void
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << value.value << std::endl;
			}, b);
		result2.get();
	}
#endif // _DEBUG
}