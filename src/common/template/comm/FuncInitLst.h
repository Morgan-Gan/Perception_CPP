#pragma once
#include <utility>
#include <functional>

namespace common_template
{
	template<class Func,class... Args>
	inline void FuncInitLst(Func&& f,Args&&... args)
	{
		std::initializer_list<int>{(f(std::forward<Args>(args)), 0)...};
	}
}