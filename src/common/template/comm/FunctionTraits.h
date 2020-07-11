#pragma once
#include <iostream>
#include <type_traits>
#include <functional>
#include <tuple>
#include <string>

namespace common_template
{
	//转换为std::function和函数指针. 
	template<typename T>
	struct function_traits;

	//普通函数.
	template<typename Ret, typename... Args>
	struct function_traits<Ret(Args...)>
	{
	public:
		enum { arity = sizeof...(Args) };

		typedef Ret function_type(Args...);
		using stl_function_type = std::function<function_type>;
		typedef Ret(*pointer)(Args...);
		typedef Ret return_type;
		typedef std::function<return_type(Args...)> FunType;
		typedef std::tuple<Args...> ArgTupleType;

		//获取指定位置的类型
		template<size_t I>
		struct args
		{
			static_assert(I < arity, "index is out of range, index must less than sizeof Args");
			using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
		};

		struct arg0
		{
			using type = typename std::tuple_element<0, std::tuple<Args...>>::type;
		};
	};

	//函数指针.
	template<typename Ret, typename... Args>
	struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

	//std::function.
	template <typename Ret, typename... Args>
	struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {};

	//member function.
#define FUNCTION_TRAITS(...)\
template <typename ReturnType, typename ClassType, typename... Args>\
struct function_traits<ReturnType(ClassType::*)(Args...) __VA_ARGS__> : function_traits<ReturnType(Args...)>{};\

	FUNCTION_TRAITS()
		FUNCTION_TRAITS(const)
		FUNCTION_TRAITS(volatile)
		FUNCTION_TRAITS(const volatile)

		//函数对象.
		template<typename Callable>
	struct function_traits : function_traits<decltype(&Callable::operator())> {};

	template <typename Function>
	typename function_traits<Function>::stl_function_type to_function(const Function& lambda)
	{
		return static_cast<typename function_traits<Function>::stl_function_type>(lambda);
	}

	template <typename Function>
	typename function_traits<Function>::stl_function_type to_function(Function&& lambda)
	{
		return static_cast<typename function_traits<Function>::stl_function_type>(std::forward<Function>(lambda));
	}

	template <typename Function>
	typename function_traits<Function>::pointer to_function_pointer(const Function& lambda)
	{
		return static_cast<typename function_traits<Function>::pointer>(lambda);
	}

#if 0
	//测试代码
	template<typename T>
	void PrintType()
	{
		std::cout << typeid(T).name() << std::endl;
	}

	float(*castfunc)(std::string, int);
	float free_function(const std::string& a, int b)
	{
		return (float)a.size() / b;
	}

	struct AA
	{
		int f(int a, int b)volatile { std::cout << "AA::f" << std::endl; return a + b; }
		int operator()(int)const { std::cout << "AA::operator()" << std::endl; return 0; }
	};

	void TestFunctionTraits()
	{
		std::function<int(int, char*)> f = [](int a, char* p) {return a; };
		PrintType<function_traits<std::function<int(int)>>::FunType>();
		PrintType<function_traits<std::function<int(int, char*)>>::args<0>>();
		PrintType<function_traits<decltype(f)>::FunType>();
		PrintType<function_traits<decltype(free_function)>::FunType>();

		PrintType<function_traits<decltype(castfunc)>::FunType>();
		PrintType<function_traits<AA>::FunType>();

		using K = function_traits<decltype(f)>::args<1>::type;
		PrintType<K>();

		using P = decltype(&AA::f);
		PrintType<function_traits<P>::FunType>();
		static_assert(std::is_same<function_traits<decltype(f)>::return_type, int>::value, "");
	}
#endif
}