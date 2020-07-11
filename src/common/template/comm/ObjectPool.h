#pragma once
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <map>
#include "NonCopyable.h"

using namespace std;

namespace common_template
{
	const int MaxObjectNum = 100;

	template<typename T>
	class ObjectPool : common_template::NonCopyable
	{
		template<typename... Args>
		using Constructor = std::function<std::shared_ptr<T>(Args...)>;
	public:
		//Ĭ�ϴ������ٸ�����
		template<typename... Args>
		void Init(size_t num, Args&&... args)
		{
			if (num <= 0 || num > MaxObjectNum)
				throw std::logic_error("object num out of range.");

			auto constructName = typeid(Constructor<Args...>).name(); //����������
			for (size_t i = 0; i < num; i++)
			{
				m_object_map.emplace(constructName, shared_ptr<T>(new T(std::forward<Args>(args)...), [this, constructName](T* p) //ɾ�����в�ֱ��ɾ�����󣬶��ǻ��յ�������У��Թ��´�ʹ��
				{
					m_object_map.emplace(std::move(constructName), std::shared_ptr<T>(p));
				}));
			}
		}

		//�Ӷ�����л�ȡһ������
		template<typename... Args>
		std::shared_ptr<T> Get()
		{
			string constructName = typeid(Constructor<Args...>).name();

			auto range = m_object_map.equal_range(constructName);
			for (auto it = range.first; it != range.second; ++it)
			{
				auto ptr = it->second;
				m_object_map.erase(it);
				return ptr;
			}

			return nullptr;
		}

	private:
		multimap<string, std::shared_ptr<T>> m_object_map;
	};

#if 0

	struct BigObject
	{
		BigObject() {}
		BigObject(int a) {}
		void Print(const string& str)
		{
			std::cout << str << std::endl;
		}
	};

	void Print(std::shared_ptr<BigObject> p,const std::string& str)
	{
		if (p != nullptr)
		{
			p->Print(str);
		}
	}

	void TestObjPool()
	{
		ObjectPool<BigObject> pool;
		pool.Init(2);//��ʼ������أ���ʼ������������
		{
			auto p = pool.Get();
			Print(p, "p");
			auto p2 = pool.Get();
			Print(p2, "p2");
		}//���������򣬶����ֱ��ػ�����

		auto p = pool.Get();
		auto p2 = pool.Get();
		Print(p, "p");
		Print(p2, "p2");

		//�����֧�����ع��캯��
		pool.Init(2, 1);
		auto p4 = pool.Get<int>();
		Print(p4, "p4");
		pool.Init(2, 1, 2);

		auto p5 = pool.Get<int,int>();
		Print(p5,"p5");
	}
#endif
}

