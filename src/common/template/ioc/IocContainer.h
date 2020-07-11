#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <Any.h>
#include <NonCopyable.h>

using namespace std;

namespace common_template
{
	class IocContainer : common_template::NonCopyable
	{
	public:
		IocContainer(void) {}
		~IocContainer(void) {}

		template<class T, typename Depend, typename... Args>
		typename std::enable_if<!std::is_base_of<T, Depend>::value>::type RegisterType(const string& strKey)
		{
			std::function<T* (Args...)> function = [](Args... args) {return new T(new Depend(args...)); };     //ͨ���հ������˲�������
			RegisterType(strKey, function);
		}

		template<class T, typename Depend, typename... Args>
		typename std::enable_if<std::is_base_of<T, Depend>::value>::type RegisterType(const string& strKey)
		{
			std::function<T* (Args...)> function = [](Args... args) {return new Depend(args...); };     //ͨ���հ������˲�������
			RegisterType(strKey, function);
		}

		template<class T, typename... Args>
		void RegisterSimple(const string& strKey)
		{
			std::function<T* (Args..)> function = [](Args... args) {return new T(args..); };
			RegisterType(strKey, function);
		}

		template<class T, typename... Args>
		T* Resolve(const string& strKey, Args... args)
		{
			auto it = m_creatorMap.find(strKey);
			if (it == m_creatorMap.end())
			{
				return nullptr;
			}

			Any resolver = it->second;
			std::function<T* (Args...)> function = resolver.AnyCast<std::function<T* (Args...)>>();

			return function(args...);
		}

		template<class T, typename... Args>
		std::shared_ptr<T> ResolveShared(const string& strKey, Args... args)
		{
			T* T = Resolve<T>(strKey, args...);
			return std::shared_ptr<T>(t);
		}

	private:
		void RegisterType(const string& strKey, Any constructor)
		{
			if (m_creatorMap.find(strKey) != m_creatorMap.end())
			{
				throw std::invalid_argument("this key has already exist!");
			}

			//ͨ��Any�����˲�ͬ���͵Ĺ�����
			m_creatorMap.emplace(strKey, constructor);
		}
	private:
		std::unordered_map<std::string, Any> m_creatorMap;
	};
}