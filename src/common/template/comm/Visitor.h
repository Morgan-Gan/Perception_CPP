#pragma once
#include <iostream>

namespace common_template
{
	template<typename... Types>
	struct Visitor;

	template<typename T, typename... Types>
	struct Visitor<T, Types..> : Visitor<Types...>
	{
		//通过using避免隐藏基类的Visit同名方法
		using Visitor<Types...>::Visit;
		virtual void Visit(const T&) = 0;
	};

	template<typename T>
	struct Visitor<T>
	{
		virtual void Visit(const T&) = 0;
	};

	struct stA;
	struct stB;

	struct Base
	{
		typedef Visitor<stA, stB> MyVisitor;
		virtual void Accept(MyVisitor&) = 0;
	};

	struct stA : Base
	{
		double val;
		void Accept(Base::MyVisitor& v)
		{
			v.Visit(*this);
		}
	};

	struct stB : Base
	{
		int val;
		void Accept(Base::MyVisitor& v)
		{
			v.Visit(*this);
		}
	};

	struct PrintVisitor : Base::MyVisitor
	{
		void Visit(const stA& a)
		{
			std::cout << "from stA: " << a.val << std::endl;
		}

		void Visit(const stB& b)
		{
			std::cout << "from stB: " << b.val << std::endl;
		}
	};

#if 0
	void TestVisitor()
	{
		PrintVisitor vis;
		stA a;
		a.val = 8.97;
		stB b;
		b.val = 9;
		Base *pBase = &a;
		pBase->Accept(vis);
		pBase = &b;
		pBase->Accept(vis);
	}
#endif
}