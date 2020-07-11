#pragma once
#include <list>
#include <functional>
#include <type_traits>

namespace common_template
{
	template<typename R = void>
	struct CommCommand
	{
	private:
	std::function<R()> m_f;

	public:
		//���ܿɵ��ö���ĺ�����װ��
		template<class F,class... Args, class = typename std::enable_if<!std::is_member_function_pointer<F>::value>::type>
		void Wrap(F&& f, Args&&... args)
		{
			//lambda���ʽ�޷�չ���ɱ����(�����Ǳ�����g++�汾������
			m_f = std::bind(f, std::forward<Args>(args)...);
		}

		//���ܳ�����Ա�����ĺ�����װ��
		template<class T,class C, class... DArgs, class P, class... Args>
		void Wrap(T(C::*f)(DArgs...) const, P && p, Args && ... args)
		{
			m_f = std::bind(f, p,std::forward<Args>(args)...);
		}

		//���ܷǳ�����Ա�����ĺ�����װ��
		template<class T, class C, class... DArgs, class P, class... Args>
		void Wrap(T(C::*f)(DArgs...), P && p, Args && ... args)
		{
			m_f = std::bind(f, p, std::forward<Args>(args)...);
		}

		R Excecute()
		{
			return m_f();
		}
	};

#if 0
	struct STA
	{
		int m_a;
		int operator()() { return m_a; }
		int operator()(int n) { return m_a + n; }
		int triple0() { return m_a * 3; }
		int triple(int a) { return m_a * 3 + a; }
		int triple1() const { return m_a * 3; }
		const int triple2(int a) const { return m_a * 3 + a; }

		void triple3() { std::cout << "" << std::endl; }
	};

	int add_one(int n)
	{
		return n + 1;
	}

	void TestWrap()
	{

		CommCommand<int> cmd;
		// free function 
		cmd.Wrap(add_one, 0);

		// lambda function
		cmd.Wrap([](int n) {return n + 1; }, 1);

		// functor 
		cmd.Wrap(bloop);
		cmd.Wrap(bloop, 4);
		STA t = { 10 };
		int x = 3;
		// member function 
		cmd.Wrap(&STA::triple0, &t);
		cmd.Wrap(&STA::triple, &t, x);
		cmd.Wrap(&STA::triple, &t, 3);

		cmd.Wrap(&STA::triple2, &t, 3);
		auto r = cmd.Excecute();

		CommCommand<> cmd1;
		cmd1.Wrap(&Bloop::triple3, &t);
		cmd1.Excecute();
	}
#endif
}