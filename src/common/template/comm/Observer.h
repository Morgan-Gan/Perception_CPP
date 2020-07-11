#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include "NonCopyable.h"
#include "FuncWrapper.h"

namespace common_template
{
	template<typename Func>
	class Events : NonCopyable
	{
	public:
		Events() {}
		~Events() {}

		//注册观察者，支持右值引用
		int Connect(Func&& f)
		{
			return Assgin(f);
		}

		//注册观察者，支持左值引用
		int Connect(const Func& f)
		{
			return Assgin(f);
		}

		//移除观察者
		void Disconnect(int key)
		{
			m_connections.erase(key);
		}

		//通知所有的观察者  
		template<typename... Args>
		void Notify(Args&&... args)
		{
			for (auto& it : m_connections)
			{
				it.second(std::forward<Args>(args)...);
			}
		}

	private:
		//保存观察者并分配观察者的编号
		template<typename F>
		int Assgin(F&& f)
		{
			int k = m_observerId++;
		}

		int m_observerId = 0;
		std::map<int, Func> m_connections;
	};

#if 0
	struct stA
	{
		int a;
		int b;
		void print(int a, int b)
		{
			cout << a << ", " << b << endl;;
		}
	};

	void print(int a, int b)
	{
		cout << a << ", " << b << endl;
	}

	int testObserver()
	{
		Events<std::function<void(int, int)>> myevent;
		auto key = myevent.Connect(print);
		stA t;
		auto lambdakey = myevent.Connect([&t](int a, int b) {t.a = a; t.b = b; });

		//std::function 注册
		std::function<void(int, int)> f = std::bind(&stA::print, &t, std::placeholders::_1, std::placeholders::_2);

		myevent.Connect(f);
		int a = 1;
		int b = 2;
		myevent.Notify(a, b);
		myevent.Disconnect(key);
		return 0;
	}
#endif
}