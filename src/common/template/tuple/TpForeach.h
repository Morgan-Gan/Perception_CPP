#pragma once
#include "TpIndexs.h"

namespace common_template
{
	namespace details 
	{
		template<typename Func, typename Last>
		void for_each_impl(Func&& f, Last&& last)
		{
			f(std::forward<Last>(last));
		}

		template<typename Func, typename First, typename ... Rest>
		void for_each_impl(Func&& f, First&& first, Rest&&...rest)
		{
			f(std::forward<First>(first));
			for_each_impl(std::forward<Func>(f), rest...);
		}

		template<typename Func, int ... Indexes, typename ... Args>
		void for_each_helper(Func&& f, IndexTuple<Indexes...>, std::tuple<Args...>&& tup)
		{
			for_each_impl(std::forward<Func>(f), std::forward<Args>(std::get<Indexes>(tup))...);
		}

	} // namespace details 

	template<typename Func, typename ... Args>
	void tp_for_each(Func&& f, std::tuple<Args...>& tup)
	{
		using namespace details;
		for_each_helper(forward<Func>(f), typename make_indexes<Args...>::type(), std::tuple<Args...>(tup));
	}

	template<typename Func, typename ... Args>
	void tp_for_each(Func&& f, std::tuple<Args...>&& tup)
	{
		using namespace details;
		for_each_helper(forward<Func>(f), typename make_indexes<Args...>::type(), forward<std::tuple<Args...>>(tup));
	}

#if 0
	struct Functor
	{
		template<typename T>
		void operator()(T& T) const {}
	};

	void TestTupleForeach()
	{
		tp_for_each(Functor(), std::make_tuple<int, double>(1, 3.3));
	}
#endif
}