#pragma once
#include <iostream>             // std::cout
#include <future>               // std::async, std::future
#include <chrono>               // std::chrono::milliseconds

namespace common_template
{
	template<typename T>
	class Task;

	template<typename R, typename...Args>
	class Task<R(Args...)>
	{
		std::function<R(Args...)> m_fn;

	public:
		typedef R return_type;

		template<typename P>
		auto Then(P&& f)->Task<typename std::result_of<P(R)>::type(Args...)>
		{
			typedef typename std::result_of<P(R)>::type ReturnType;
			auto func = std::move(m_fn);
			return Task<ReturnType(Args...)>([func, &f](Args&&... args)
			{
				std::future<R> lastf = std::async(func, std::forward<Args>(args)...);
				return std::async(f, lastf.get()).get();
			});
		}

		Task(std::function<R(Args...)>&& f) : m_fn(std::move(f)) {}
		Task(std::function<R(Args...)>& f) : m_fn(f) {}

		~Task() {}

		//�ȴ��첽�������
		void Wait()
		{
			std::async(m_fn).wait();
		}

		//��ȡ�첽�����Ľ��
		template<typename... TArgs>
		R Get(TArgs &&... args)
		{
			return std::async(m_fn, std::forward<TArgs>(args)...).get();
		}

		//�����첽����
		std::shared_future<R> Run()
		{
			return std::async(m_fn);
		}
	};

	void TestTaskThen()
	{
		Task<int()> t([]() {return 32; });
		auto r1 = t.Then([](int result) {std::cout << result << std::endl; return result + 3; }).Then([](int result) {std::cout << result << std::endl; return result + 3; }).Get();
		std::cout << r1 << std::endl;

		Task<int(int)> t1([](int i) {return i; });
		t1.Then([](int result) {return std::to_string(result); }).Then([](const std::string& str) {std::cout << str << std::endl; return 0; }).Get(5);

		Task<std::string(std::string)> t2([](std::string str) {return str; });
		std::string in = "test";
		auto r2 = t2.Then([](const std::string& str) {std::cout << str.c_str() << std::endl; return str + " ok"; }).Get("test");
		std::cout << r2 << std::endl;
	}
}