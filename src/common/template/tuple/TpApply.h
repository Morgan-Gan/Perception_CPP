#pragma once
#include "TpIndexs.h"

namespace common_template
{
	template<class F, class ... Args, int ... Indexes>
	typename std::result_of<F(Args...)>::type apply_helper(F&& pf, IndexTuple< Indexes... >&& in, tuple<Args...>&& tup)
	{
		return forward<F>(pf)(forward<Args>(get<Indexes>(tup))...);
	}

	template<class F, class ... Args>
	typename std::result_of<F(Args...)>::type apply(F&& pf, const tuple<Args...>&  tup)
	{
		return apply_helper(forward<F>(pf), typename make_indexes<Args...>::type(), tuple<Args...>(tup));
	}

	template<class F, class ... Args>
	typename std::result_of<F(Args...)>::type apply(F&& pf, tuple<Args...>&&  tup)
	{
		return apply_helper(forward<F>(pf), typename make_indexes<Args...>::type(), forward<tuple<Args...>>(tup));
	}

	template<class F, class ... Args>
	typename std::result_of<F(Args...)>::type apply2(F&& pf, const tuple<Args...>&  tup)
	{
		return apply_helper(forward<F>(pf), typename make_indexes2<sizeof ...(Args)>::type(), tuple<Args...>(tup));
	}

	template<class F, class ... Args>
	typename std::result_of<F(Args...)>::type apply2(F&& pf, tuple<Args...>&&  tup)
	{
		return apply_helper(forward<F>(pf), typename make_indexes2<sizeof ...(Args)>::type(), forward<tuple<Args...>>(tup));
	}

	template<class F, class ... Args>
	typename std::result_of<F(Args...)>::type apply3(F&& pf, const tuple<Args...>&  tup)
	{
		return apply_helper(forward<F>(pf), typename make_indexes3<sizeof ...(Args), 0>::type(), tuple<Args...>(tup));
	}

	template<class F, class ... Args>
	typename std::result_of<F(Args...)>::type apply3(F&& pf, tuple<Args...>&&  tup)
	{
		return apply_helper(forward<F>(pf), typename make_indexes3<sizeof ...(Args), 0>::type(), forward<tuple<Args...>>(tup));
	}

#if 0
	namespace detail
	{
		//根据值获取tuple的索引位置
		template<size_t N, typename Tuple, typename T>
		static std::enable_if<std::is_convertible<std::tuple_element_t<N, Tuple>, T>::value, bool> equal_val(const Tuple& tp, const T& t)
		{
			return std::get<N>(tp) == t;
		}

		template<size_t N, typename Tuple, typename T>
		static std::enable_if<!std::is_convertible<std::tuple_element_t<N, Tuple>, T>::value, bool> equal_val(const Tuple& tp, const T& t)
		{
			return false;
		}

		template<int I, typename T, typename... Args>
		struct find_index
		{
			static int call(std::tuple<Args...> const& t, T&& val)
			{
				using U = std::remove_reference_t < std::remove_cv_t<T>>;
				using V = std::tuple_element_t<I - 1, std::tuple<Args...>>;
				return (std::is_convertible<U, V>::value&&equal_val<I - 1>(t, val)) ? I - 1 :
					find_index<I - 1, T, Args...>::call(t, std::forward<T>(val));
			}
		};

		template<typename T, typename... Args>
		struct find_index<0, T, Args...>
		{
			//递归终止，如找到 返回0，否则返回-1
			static int call(std::tuple<Args...> const& t, T&& val)
			{
				using U = std::remove_reference_t < std::remove_cv_t<T>>;
				using V = std::tuple_element_t<0, std::tuple<Args...>>;
				return (std::is_convertible<U, V>::value&&equal_val<0>(t, val)) ? 0 : -1;
			}
		};
		 
		//根据索引获取元素值
		template<size_t k,typename Tuple>
		typename std::enable_if<(k == std::tuple_size<Tuple>::value)>::type GetArgByIndex(size_t index,Tuple& tp)
		{
			throw std::invalid_argument("arg index out of range");
		}

		template<size_t k = 0,typename Tuple>
		typename std::enable_if<(k < std::tuple_size<Tuple>::value)>::type GetArgByIndex(size_t index, Tuple& tp)
		{
			if (k == index)
			{
				std::cout << std::get<k>(tp) << std::endl;
			}
			else
			{
				GetArgByIndex<k + 1>(index, tp);
			}
		}
	}

	template<typename T, typename... Args>
	int find_index(std::tuple<Args...> const& t, T&& val)
	{
		return detail::find_index<sizeof...(Args), T, Args...>::call(t, std::forward<T>(val));
	}
#endif
}