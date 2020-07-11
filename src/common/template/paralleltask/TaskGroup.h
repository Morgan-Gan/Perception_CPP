#pragma once
#include <vector>
#include <map>
#include <string>
#include <future>
#include "Any.h"
#include "NonCopyable.h"
#include "Task.h"
#include "Variant.h"
#include "FunctionTraits.h"

namespace common_template
{
	class TaskGroup : common_template::NonCopyable
	{
		using RetVariant = common_template::Variant<int, std::string, double, short, unsigned int>;

	public:
		TaskGroup() {}
		~TaskGroup() {}

		template<typename R, typename = typename std::enable_if<!std::is_same<R, void>::value>::type>
		void Run(common_template::Task<R()>&& task)
		{
			m_group.emplace(R(), std::move(task).Run());
		}

		template<typename P>
		void Run(P&& f)
		{
			typedef typename std::result_of<P()>::type ReturnType;
			typedef typename common_template::Task<ReturnType()> T;
			Run(T(std::forward<P>(f)));
		}

		template<typename U, typename... Funs>
		void Run(U&& first, Funs&&... rest)
		{
			Run(std::forward<U>(first));
			Run(std::forward<Funs>(rest)...);
		}

		void Wait()
		{
			for (auto it = m_group.begin(); it != m_group.end(); it++)
			{
				auto vrt = it->first;
				vrt.Visit([&](int a) {FutureGet<int>(it->second); }, [&](double b) {FutureGet<double>(it->second); }, [&](std::string v) {FutureGet<std::string>(it->second); }, [&](short v) {FutureGet<short>(it->second); }, [&](unsigned int v) {FutureGet<unsigned int>(it->second); });
			}
		}

	private:
		template<typename P>
		void FutureGet(common_template::Any& f)
		{
			f.AnyCast<std::shared_future<P>>().get();
		}

	private:
		std::multimap<RetVariant, common_template::Any> m_group;
	};

	/*���е�������ɺ󷵻�*/
	template<typename Range>
	common_template::Task<std::vector<typename Range::value_type::return_type>()> WhenAll(Range& range)
	{
		typedef typename Range::value_type::return_type ReturnType;
		auto task = [&range]()
		{
			std::vector<std::shared_future<ReturnType>> fv;
			for (auto & var : range)
			{
				fv.emplace_back(var.Run());
			}

			std::vector<ReturnType> v;
			for (auto& item : fv)
			{
				v.emplace_back(item.get());
			}

			return v;
		};
		return common_template::Task<std::vector<ReturnType>()>(task);
	}

	/*����һ����������ͷ���*/
	namespace Detail
	{
		template<typename R>
		struct RangeTrait
		{
			typedef R Type;
		};

		template<typename R>
		struct RangeTrait<std::shared_future<R>>
		{
			typedef R Type;
		};

		template<typename Range>
		std::vector<std::shared_future<typename Range::value_type::return_type>> TransForm(Range& range)
		{
			typedef typename Range::value_type::return_type ReturnType;
			std::vector<std::shared_future<ReturnType>> fv;

			for (auto & var : range)
			{
				fv.emplace_back(var.Run());
			}

			return fv;
		}

		template<typename Range>
		std::pair<int, typename RangeTrait<typename Range::value_type>::Type> GetAnyResultPair(Range& fv)
		{
			size_t size = fv.size();
			while (true)
			{
				for (size_t i = 0; i < size; i++)
				{
					std::future_status s32Status = fv[i].wait_for(std::chrono::microseconds(1));
					if (fv[i].wait_for(std::chrono::microseconds(1)) == std::future_status::timeout)
					{
						return std::make_pair(i, fv[i].get());
					}
				}
			}
		}
	}

	void PrintHead()
	{
		std::cout << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	template<typename Range>
	common_template::Task<std::pair<int, typename Range::value_type::return_type>()> WhenAny(Range& range)
	{
		auto task = [&range]
		{
			using namespace Detail;
			auto&& var = TransForm(range);
			return GetAnyResultPair(var);
		};

		typedef typename Range::value_type::return_type ReturnType;
		return common_template::Task<std::pair<int, ReturnType>()>(task);
	}


	void TestWhenAll()
	{
		std::vector<common_template::Task<int()>> v =
		{
			common_template::Task<int()>([]()->int {PrintHead(); std::this_thread::sleep_for(std::chrono::seconds(1)); return 1; }),
			common_template::Task<int()>([]()->int {PrintHead(); return 2; }),
			common_template::Task<int()>([]()->int {PrintHead(); return 3; }),
			common_template::Task<int()>([]()->int {PrintHead(); return 4; })
		};
		std::cout << "when all" << std::endl;
		WhenAll(v).Then([](std::vector<int> results) {
			std::cout << "The sum is :" << std::accumulate(begin(results), end(results), 0) << std::endl;
			return 0;
		}).Wait();
	}

	void TestWhenAny()
	{
		std::vector<common_template::Task<int()>> v =
		{
			common_template::Task<int()>([] {PrintHead(); std::this_thread::sleep_for(std::chrono::seconds(1)); return 1; }),
			common_template::Task<int()>([] {PrintHead(); return 2; }),
			common_template::Task<int()>([] {PrintHead(); return 3; }),
			common_template::Task<int()>([] {PrintHead(); return 4; })
		};

		std::cout << "when any" << std::endl;

		WhenAny(v).Then([](const std::pair<int, int>& results)
		{
			std::cout << " index " << results.first << " result " << results.second << std::endl;
			return results.second;
		}).Then([](int result)
		{
			std::cout << "any result:" << result << std::endl;
			return 0;
		}).Get();
	}

	void TestTaskGroup()
	{
		TaskGroup g;
		std::function<double()> f1 = []()->double {std::cout << "ok1" << std::endl; return 9.8; };
		std::function<int()> f2 = []()->int {std::cout << "ok2" << std::endl; return 2; };
		std::function<int()> f3 = []()->int {std::cout << "ok3" << std::endl; return 3; };
		g.Run(f1, f2, f3);
		g.Wait();
	}
}