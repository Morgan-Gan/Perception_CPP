#pragma once
#include "TaskGroup.h"

namespace common_template
{
	template<class Iterator, class Function>
	void ParallelForeach(Iterator& begin, Iterator& end, Function& func)
	{
		auto partNum = std::thread::hardware_concurrency();
		auto blockSize = std::distance(begin, end) / partNum;
		Iterator last = begin;
		if (blockSize > 0)
		{
			std::advance(last, (partNum - 1) * blockSize);
		}
		else
		{
			last = end;
			blockSize = 1;
		}

		std::vector<std::future<void>> futures;
		//ǰ���N - 1�������
		for (; begin != last; std::advance(begin, blockSize))
		{
			futures.emplace_back(std::async([begin, blockSize, &func]
			{
				std::for_each(begin, begin + blockSize, func);
			}));
		}

		//���һ�������
		futures.emplace_back(std::async([&begin, &end, &func]
		{
			std::for_each(begin, end, func);
		}));

		std::for_each(futures.begin(), futures.end(), [](std::future<void>&futuer)
		{
			futuer.get();
		});
	}

	template<typename... Funs>
	void ParallelInvoke(Funs&&... rest)
	{
		wdp_aas::aas_paralleltask::TaskGroup group;
		group.Run(std::forward<Funs>(rest)...);
		group.Wait();
	}

	template<typename Range, typename ReduceFunc>
	typename Range::value_type ParallelReduce(Range& range, typename Range::value_type &init, ReduceFunc reduceFunc)
	{
		return ParallelReduce<Range, ReduceFunc>(range, init, reduceFunc, reduceFunc);
	}

	template<typename Range, typename RangeFunc, typename ReduceFunc>
	typename Range::value_type ParallelReduce(Range& range, typename Range::value_type &init, RangeFunc& rangeFunc, ReduceFunc& reduceFunc)
	{
		auto partNum = std::thread::hardware_concurrency();
		auto begin = std::begin(range);
		auto end = std::end(range);
		auto blocksSize = std::distance(begin, end) / partNum;
		typename Range::iterator last = begin;
		if (blocksSize > 0)
		{
			std::advance(last, (partNum - 1) * blocksSize);
		}
		else
		{
			last = end;
			blocksSize = 1;
		}

		typedef typename Range::value_type ValueType;
		std::vector<std::future<ValueType>> futures;

		//first p - 1 groups
		for (; begin != last; std::advance(begin, blocksSize))
		{
			futures.emplace_back(std::async([begin, &init, blocksSize, &rangeFunc]
			{
				return rangeFunc(begin, begin + blocksSize, init);
			}));
		}

		//last group
		futures.emplace_back(std::async([&begin, &end, &init, &rangeFunc]
		{
			return rangeFunc(begin, end, init);
		}));

		std::vector<ValueType> results;
		std::for_each(futures.begin(), futures.end(), [&results](std::future<ValueType>& futuer)
		{
			results.emplace_back(futuer.get());
		});

		return reduceFunc(results.begin(), results.end(), init);
	}

	void TestFindString()
	{
		std::vector<std::string> v;
		v.reserve(1000000);
		for (int i = 0; i < 1000000; i++)
		{
			v.emplace_back(std::to_string(i + 1));
		}

		auto f = [](const std::vector<std::string>::iterator& begin, const std::vector<std::string>::iterator& end, const std::string& val)
		{
			return *std::max_element(begin, end, [](const std::string& str1, const std::string& str2)
			{
				return str1.length() < str2.length();
			});
		};

		std::string init = "";
		auto r = ParallelReduce(v, init, f, f);

		std::cout << "ParallelAlg Process Result:" << r << std::endl;
	}
}