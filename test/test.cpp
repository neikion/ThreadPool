#include "test.h"
#include<iostream>
#include<chrono>

namespace ThreadPoolTest {
	using namespace ThreadPoolSpace;
	class testclass {
	public:
		int value;
		testclass() :value(0) {
			std::cout << "default" << std::endl;
		}
		testclass(int invalue) : value(invalue) {
			std::cout << "default" << std::endl;
		};
		testclass(const testclass& invalue) : value(invalue.value) {
			std::cout << "copy cons" << std::endl;
		}
		testclass(const testclass&& invalue) noexcept : value(invalue.value) {
			std::cout << "move cons" << std::endl;
		}
		testclass& operator=(testclass& value) {
			std::cout << "= lvalue" << std::endl;
			this->value = value.value;
			return *this;
		}
		testclass& operator=(testclass&& value) noexcept {
			std::cout << "= rvalue" << std::endl;
			value.value = 0;
			this->value = value.value;
			return *this;
		}
	};
	/// <summary>
	/// return void arg void
	/// </summary>
	void case1(ThreadPool& tp) {
		std::future<void> result = tp.push(
			[]()->void
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		);
		result.get();
		std::cout << "case1 complate" << std::endl << std::endl;
	}
	/// <summary>
	/// return value arg void
	/// </summary>
	void case2(ThreadPool& tp) {
		std::future<int> result = tp.push(
			[]()->int
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return 1;
			}
		);
		std::cout << result.get()<<std::endl;
		std::cout << "case2 complate" << std::endl << std::endl;

	}
	/// <summary>
	/// return class(copy) arg void
	/// </summary>
	void case3(ThreadPool& tp) {
		std::future<testclass> result = tp.push(
			[]()->testclass
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return testclass(1);
			}
		);
		result.get();
		std::cout << "case3 complate" << std::endl << std::endl;
	}
	/// <summary>
	/// return class(rvalue ref) arg void
	/// </summary>
	void case4(ThreadPool& tp) {
		//C2338	static_assert failed: 'T in future<T> must meet the Cpp17Destructible requirements (N4950 [futures.unique.future]/4).'
		/*
		return 시 testclass&&는 rvalue 참조를 전달하는 것으로 보인다.
		이는 함수가 끝날 때 객체가 파괴되므로 참조는 전달되더라도 객체가 파괴되기 때문에 의미가 없다.
		즉, 함수가 끝날 시 댕글링 참조인 객체를 반환할 가능성이 있어 막아둔 것으로 보인다.
		또한 고유한 future는 rvalue로 값을 반환하므로, rvalue로 반환할 필요 없이 그냥 값으로 반환하면 된다.

		std::future<testclass&&> result = tp.push(
			[]()->testclass&&
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return testclass();
			}
		);
		result.get();
		*/
	}
	/// <summary>
	/// return class(copy) arg class(copy)
	/// </summary>
	void case5(ThreadPool& tp) {
		testclass anyclass(1);
		std::future<testclass> result = tp.push(
			[](testclass value)->testclass
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return value;
			}, anyclass
		);
		std::cout<<result.get().value<<std::endl;
		std::cout << "case5 complate" << std::endl << std::endl;
	}
	/// <summary>
	/// return class(lvalue ref) arg(lvalue ref)
	/// </summary>
	void case6(ThreadPool& tp) {
		testclass anyclass(1);
		std::future<testclass&> result = tp.push(
			[](testclass& value)->testclass&
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return value;
			}, anyclass
		);
		std::cout << result.get().value << std::endl;
		std::cout << "case6 complate" << std::endl << std::endl;
	}
	/// <summary>
	/// return void arg(rvalue ref)
	/// </summary>
	void case7(ThreadPool& tp) {
		testclass anyclass(1);
		std::future<void> result = tp.push([](testclass&& value) ->void
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << value.value << std::endl;
			}, std::move(anyclass));
		result.get();
		std::cout << "case7 complate" << std::endl << std::endl;
	}
	/// <summary>
	/// return void arg multi
	/// </summary>
	void case8(ThreadPool& tp) {
		testclass a(1), b(2);
		std::future<void> result = tp.push([](testclass& value1,testclass&& value2) ->void
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << value1.value << std::endl;
				std::cout << value2.value << std::endl;
			}, a,std::move(b));
		result.get();
		std::cout << "case8 complate" << std::endl << std::endl;
	}

	void ThreadMain() {
		
		ThreadPoolSpace::ThreadPool tp{};
		case1(tp);
		case2(tp);
		case3(tp);
		case4(tp);
		case5(tp);
		case6(tp);
		case7(tp);
		case8(tp);
	}
}