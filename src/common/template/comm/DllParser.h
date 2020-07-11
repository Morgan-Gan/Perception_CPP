#pragma once
#ifndef _WIN32
#include <dlfcn.h>
#else
#include <Windows.h>
#endif
#include <stdexcept>
#include <string>
#include <map>
#include <functional>
using namespace std;

namespace common_template
{
	class DllParser
	{
	public:

		DllParser() :m_hMod(nullptr)
		{
		}

		~DllParser()
		{
			UnLoad();
		}

		bool Load(const string& dllPath)
		{
#ifndef _WIN32
			m_hMod = dlopen(dllPath.c_str(), RTLD_NOW);
#else
			m_hMod = LoadLibraryA(dllPath.data());
#endif
			if (nullptr == m_hMod)
			{
				return false;
			}

			return true;
		}

		bool UnLoad()
		{
			if (nullptr == m_hMod)
			{
				return true;
			}

#ifndef _WIN32
			auto b = dlclose(m_hMod);
#else
			auto b = FreeLibrary(m_hMod);
#endif 
			m_hMod = nullptr;
			return true;
		}

		template <typename T>
		std::function<T> GetFunction(const string& funcName)
		{
			auto it = m_map.find(funcName);
			if (it == m_map.end())
			{
#ifndef _WIN32
				auto addr = dlsym(m_hMod, funcName.c_str());
#else
				auto addr = GetProcAddress(m_hMod, funcName.c_str());
#endif
				if (!addr)
				{
					return nullptr;
				}
				m_map.insert(std::make_pair(funcName, addr));
				it = m_map.find(funcName);
			}

			return std::function<T>((T*)(it->second));
		}

		template <typename T, typename... Args>
		typename std::result_of<std::function<T>(Args...)>::type ExcecuteFunc(const string& funcName, Args&&... args)
		{
			auto f = GetFunction<T>(funcName);
			if (nullptr == f)
			{
				string s = "can not find this function " + funcName;
				throw std::runtime_error(s);
			}

			return f(std::forward<Args>(args)...);
		}

	private:
#ifndef _WIN32
		void* m_hMod;
		std::map<string, void*> m_map;
#else
		HMODULE m_hMod;
		std::map<string, FARPROC> m_map;
#endif	
	};
}