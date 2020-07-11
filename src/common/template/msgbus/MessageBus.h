#pragma once
#include <string>
#include <functional>
#include <map>
#include "comm/Any.h"
#include "comm/FunctionTraits.h"
#include "comm/NonCopyable.h"

namespace common_template
{
	class MessageBus : common_template::NonCopyable
	{
	public:
		//ע����Ϣ
		template<typename F>
		void Attach(F&& f, const std::string& strTopic = "")
		{
			auto func = common_template::to_function(std::forward<F>(f));
			Add(strTopic, std::move(func));
		}

		//������Ϣ
		template<typename R>
		void SendReq(const std::string& strTopic = "")
		{
			using function_type = std::function<R()>;
			std::string strMsgType = strTopic + typeid(function_type).name();
			auto range = m_map.equal_range(strMsgType);
			for (Iterater it = range.first; it != range.second; ++it)
			{
				auto f = it->second.AnyCast<function_type>();
				f();
			}
		}
		template<typename R, typename... Args>
		void SendReq(Args&&... args, const std::string& strTopic = "")
		{
			using function_type = std::function<R(Args...)>;
			std::string strMsgType = strTopic + typeid(function_type).name();
			auto range = m_map.equal_range(strMsgType);
			for (Iterater ite = range.first; ite != range.second; ++ite)
			{
				auto f = ite->second.AnyCast<function_type>();
				f(std::forward<Args>(args)...);
			}
		}

		//�Ƴ�ĳ������,��Ҫ�������Ϣ����
		template<typename R, typename... Args>
		void Remove(const std::string& strTopic = "")
		{
			using function_type = std::function<R(Args...)>;
			std::string strMsgType = strTopic + typeid(function_type).name();
			int count = m_map.count(strMsgType);
			auto range = m_map.equal_range(strMsgType);
			m_map.erase(range.first, range.second);
		}

	private:
		template<typename F>
		void Add(const std::string& strTopic, F&& f)
		{
			std::string strMsgType = strTopic + typeid(F).name();
			m_map.emplace(std::move(strMsgType), std::forward<F>(f));
		}

	private:
		std::multimap<std::string, common_template::Any> m_map;
		using Iterater = std::multimap<std::string, common_template::Any>::iterator;
	};

#if 0
	const std::string Topic = "Drive";
	MessageBus Megbus;

	struct Subject
	{
		void SendReq(const std::string& topic)
		{
			Megbus.SendReq<void, int>(50, Topic);
		}
	};

	struct Car
	{
		Car()
		{
			Megbus.Attach([this](int speed) {Driver(speed); }, Topic);
		}

		void Driver(int speed)
		{
			std::cout << "Car drive" << speed << std::endl;
		}
	};

	struct Bus
	{
		Bus()
		{
			Megbus.Attach([this](int speed) {Driver(speed); }, Topic);
		}

		void Driver(int speed)
		{
			std::cout << "Bus drive" << speed << std::endl;
		}
	};

	struct Truck
	{
		Truck()
		{
			Megbus.Attach([this](int speed) {Driver(speed); });
		}

		void Driver(int speed)
		{
			std::cout << "Truck drive" << speed << std::endl;
		}
	};

	//���Դ���
	void TestMsgBus()
	{
		//ע����Ϣ
		Megbus.Attach([](int a) {std::cout << "no reference" << a << std::endl; });
		Megbus.Attach([](int& a) {std::cout << "lvalue reference" << a << std::endl; });
		Megbus.Attach([](int&& a) {std::cout << "rvalue reference" << a << std::endl; });
		Megbus.Attach([](const int& a) {std::cout << "const lvalue reference" << a << std::endl; });
		Megbus.Attach([](int a) {std::cout << "no reference has return value and key" << a << std::endl; return a; }, "a");

		//������Ϣ
		int i = 2;
		Megbus.SendReq<void, int>(2);
		Megbus.SendReq<int, int>(2, "a");
		Megbus.SendReq<void, int&>(i);
		Megbus.SendReq<void, const int&>(2);
		Megbus.SendReq<void, int&&>(2);

		//�Ƴ���Ϣ
		Megbus.Remove<void, int>();
		Megbus.Remove<int, int>("a");
		Megbus.Remove<void, int&>();
		Megbus.Remove<void, const int&>();
		Megbus.Remove<void, int&&>();

		//������Ϣ
		Megbus.SendReq<void, int>(2);
		Megbus.SendReq<int, int>(2, "a");
		Megbus.SendReq<void, int&>(i);
		Megbus.SendReq<void, const int&>(2);
		Megbus.SendReq<void, int&&>(2);
	}

	void TestBus()
	{
		TestMsgBus();

		Car car;
		Bus local_bus;
		Truck truck;

		Subject subject;
		subject.SendReq(Topic);
		subject.SendReq("");

		Megbus.Remove<void, int>();
		subject.SendReq("");
	}
#endif
}