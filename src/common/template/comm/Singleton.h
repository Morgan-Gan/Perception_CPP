#pragma once
#include <utility>
#include "NonCopyable.h"

namespace common_template
{
	template<typename T>
	class CSingleton : NonCopyable
	{
	public:
		template<typename... Args>
		static T& GetInstance(Args&&... args)
		{
			static T*  value_ = new T(std::forward<Args>(args)...);
			return *value_;
		}

	protected:
		CSingleton();
		~CSingleton();
	};

	template<typename T>
	inline CSingleton<T>::~CSingleton()
	{
	}

	template<typename T>
	inline CSingleton<T>::CSingleton()
	{
	}
}