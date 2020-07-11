#pragma once
#include <typeindex>
#include <iostream>
#include <type_traits>
#include <tuple>
#include <functional>
#include "FunctionTraits.h"

namespace common_template
{
	//��ȡ��������
	template <size_t arg, size_t... rest>
	struct IntegerMax;

	template <size_t arg>
	struct IntegerMax<arg> : std::integral_constant<size_t, arg>
	{
		/*
		static const size_t value = arg;
		enum{value = arg};
		*/
	};

	//��ȡ����align
	template <size_t arg1, size_t arg2, size_t... rest>
	struct IntegerMax<arg1, arg2, rest...> : std::integral_constant<size_t, arg1 >= arg2 ? IntegerMax<arg1, rest...>::value :
		IntegerMax<arg2, rest...>::value >
	{
		/*
		static const size_t value = arg1 >= arg2 ? static_max<arg1, others...>::value :
		static_max<arg2, others...>::value;
		*/
	};


	template<typename... Args>
	struct MaxAlign : std::integral_constant<int, IntegerMax<std::alignment_of<Args>::value...>::value> {};

	/*
	template<typename T, typename... Args>
	struct MaxAlign : std::integral_constant<int,
	(std::alignment_of<T>::value >MaxAlign<Args...>::value ? std::alignment_of<T>::value : MaxAlign<Args...>::value) >
	{};
	template<typename T>
	struct MaxAlign<T> : std::integral_constant<int, std::alignment_of<T>::value >{};
	*/

	//�Ƿ����ĳ������
	template < typename T, typename... List >
	struct Contains : std::true_type {};

	template < typename T, typename Head, typename... Rest >
	struct Contains<T, Head, Rest...>
		: std::conditional< std::is_same<T, Head>::value, std::true_type, Contains<T, Rest... >> ::type {};

	template < typename T >
	struct Contains<T> : std::false_type {};

	//��ȡ��һ��T������λ��
	// Forward
	template<typename Type, typename... Types>
	struct GetLeftSize;

	// Declaration
	template<typename Type, typename First, typename... Types>
	struct GetLeftSize<Type, First, Types...> : GetLeftSize<Type, Types...>
	{
	};

	// Specialized
	template<typename Type, typename... Types>
	struct GetLeftSize<Type, Type, Types...> : std::integral_constant<int, sizeof...(Types)>
	{
		//static const int ID = sizeof...(Types);
	};

	template<typename Type>
	struct GetLeftSize<Type> : std::integral_constant<int, -1>
	{
		//static const int ID = -1;
	};

	template<typename T, typename... Types>
	struct Index : std::integral_constant<int, sizeof...(Types)-GetLeftSize<T, Types...>::value - 1> {};

	//����������ȡ����λ�õ�����
	// Forward declaration
	template<int index, typename... Types>
	struct IndexType;

	// Declaration
	template<int index, typename First, typename... Types>
	struct IndexType<index, First, Types...> : IndexType<index - 1, Types...>
	{
	};

	// Specialized
	template<typename First, typename... Types>
	struct IndexType<0, First, Types...>
	{
		typedef First DataType;
	};

	template<typename... Args>
	struct VariantHelper;

	template<typename T, typename... Args>
	struct VariantHelper<T, Args...>
	{
		inline static void Destroy(std::type_index id, void * data)
		{
			if (id == std::type_index(typeid(T)))
				//((T*) (data))->~T();
				reinterpret_cast<T*>(data)->~T();
			else
				VariantHelper<Args...>::Destroy(id, data);
		}

		inline static void move(std::type_index old_t, void * old_v, void * new_v)
		{
			if (old_t == std::type_index(typeid(T)))
				new (new_v)T(std::move(*reinterpret_cast<T*>(old_v)));
			else
				VariantHelper<Args...>::move(old_t, old_v, new_v);
		}

		inline static void copy(std::type_index old_t, const void * old_v, void * new_v)
		{
			if (old_t == std::type_index(typeid(T)))
				new (new_v)T(*reinterpret_cast<const T*>(old_v));
			else
				VariantHelper<Args...>::copy(old_t, old_v, new_v);
		}
	};

	template<>
	struct VariantHelper<>
	{
		inline static void Destroy(std::type_index id, void * data) {  }
		inline static void move(std::type_index old_t, void * old_v, void * new_v) { }
		inline static void copy(std::type_index old_t, const void * old_v, void * new_v) { }
	};

	template<typename... Types>
	class Variant
	{
		typedef VariantHelper<Types...> Helper_t;

		enum
		{
			data_size = IntegerMax<sizeof(Types)...>::value,
			//align_size = IntegerMax<alignof(Types)...>::value
			align_size = MaxAlign<Types...>::value //ctp����alignof, Ϊ�˼����ô˰汾
		};
		using data_t = typename std::aligned_storage<data_size, align_size>::type;

	public:
		template<int index>
		using IndexType = typename IndexType<index, Types...>::DataType;

		Variant(void) :m_typeIndex(typeid(void))//, m_index(-1)
		{
		}

		~Variant()
		{
			Helper_t::Destroy(m_typeIndex, &m_data);
		}

		Variant(Variant<Types...>&& old) : m_typeIndex(old.m_typeIndex)
		{
			Helper_t::move(old.m_typeIndex, &old.m_data, &m_data);
		}

		Variant(const Variant<Types...>& old) : m_typeIndex(old.m_typeIndex)
		{
			Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
		}

		Variant& operator=(const Variant& old)
		{
			Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
			m_typeIndex = old.m_typeIndex;
			return *this;
		}

		Variant& operator=(Variant&& old)
		{
			Helper_t::move(old.m_typeIndex, &old.m_data, &m_data);
			m_typeIndex = old.m_typeIndex;
			return *this;
		}

		template <class T, class = typename std::enable_if<Contains<typename std::remove_reference<T>::type, Types...>::value>::type>
		Variant(T&& value) : m_typeIndex(typeid(void))
		{
			Helper_t::Destroy(m_typeIndex, &m_data);
			typedef typename std::remove_reference<T>::type U;
			new(&m_data) U(std::forward<T>(value));
			m_typeIndex = std::type_index(typeid(T));
		}

		template<typename T>
		bool Is() const
		{
			return (m_typeIndex == std::type_index(typeid(T)));
		}

		bool Empty() const
		{
			return m_typeIndex == std::type_index(typeid(void));
		}

		std::type_index Type() const
		{
			return m_typeIndex;
		}

		template<typename T>
		typename std::decay<T>::type& Get()
		{
			using U = typename std::decay<T>::type;
			if (!Is<U>())
			{
				std::cout << typeid(U).name() << " is not defined. " << "current type is " <<
					m_typeIndex.name() << std::endl;
				throw std::bad_cast();
			}

			return *(U*)(&m_data);
		}

		template<typename T>
		int GetIndexOf()
		{
			return Index<T, Types...>::value;
		}

		template<typename F>
		void Visit(F&& f)
		{
			using T = typename wdp_aas::aas_template::function_traits<F>::arg0::type;
			if (Is<T>())
			{
				f(Get<T>());
			}
		}

		template<typename F, typename... Rest>
		void Visit(F&& f, Rest&&... rest)
		{
			using T = typename wdp_aas::aas_template::function_traits<F>::arg0::type;
			if (Is<T>())
			{
				Visit(std::forward<F>(f));
			}
			else
			{
				Visit(std::forward<Rest>(rest)...);
			}
		}

		bool operator==(const Variant& rhs) const
		{
			return m_typeIndex == rhs.m_typeIndex;
		}

		bool operator<(const Variant& rhs) const
		{
			return m_typeIndex < rhs.m_typeIndex;
		}

	private:
		data_t m_data;
		std::type_index m_typeIndex;  //����ID
	};

#if 0
	void TestVariant()
	{
		typedef Variant<double, int, std::string, int> cv;

		cv v = std::string("hello world");

		//ͨ��һ��lambda����vairant
		v.Visit([&](int i) {std::cout << "int: " << i << std::endl; },
			[&](double i) {std::cout << "double: " << i << std::endl; },
			[](std::string i) {std::cout << "std::string: " << i << std::endl; },
			[&](int i) {std::cout << "short: " << i << std::endl; }
		);

		int i = 0;
	}
#endif
}