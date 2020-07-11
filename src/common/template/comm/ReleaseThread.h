#pragma once

namespace common_template
{
	template<class T>
	inline void DelThread(T&& head)
	{
		std::forward<T>(head).reset();
	}

	template<typename... Args>
	inline void ReleaseThread(Args&&... args)
	{
		std::initializer_list<int>{(DelThread(args), 0)...};
	}
}