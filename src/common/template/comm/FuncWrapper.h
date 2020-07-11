#pragma once
#include <utility>

namespace common_template
{
	template<class Function, class... Args>
	inline auto FuncWrapper(Function&& f, Args&&... args) -> decltype(f(std::forward<Args>(args)...))
	{
		return f(std::forward<Args>(args)...);
	}

#if 0
	int testFuncWrapper()
	{
		return 0;
	}
#endif
}