#pragma once
#include <chrono>
#include <ctime>
#include <ratio>
#include <functional>

namespace common_template
{
    template<typename Duration>
	class TimeWatcher
	{
		typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
		typedef std::function<void(int64_t)> CallBackFun;

	public:
		TimeWatcher(const unsigned int timeout) :m_TimeOut(timeout) {}
		TimeWatcher(const unsigned int timeout, CallBackFun cb) :m_TimeOut(timeout), m_CallBack(cb) {}

		~TimeWatcher()
		{
			if (IsTimeOut() && m_CallBack)
			{
				m_CallBack(GetTimeOutCount());
			}
		}

	public:
		inline bool IsTimeOut(const timePoint* pPoint = nullptr)
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> _now = std::chrono::high_resolution_clock::now();
			if (nullptr == pPoint)
			{
				return std::chrono::duration_cast<Duration>(_now - _start) >= Duration(m_TimeOut);
			}
			else
			{
				return std::chrono::duration_cast<Duration>(_now - *pPoint) >= Duration(m_TimeOut);
			}
		}

		inline int64_t GetTimeOutCount()
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> _now = std::chrono::high_resolution_clock::now();
			auto duration_s = std::chrono::duration_cast<Duration>(_now - _start);
			return duration_s.count();
		}

		//reset start time
		inline void ResetStartTime()
		{
			_start = std::chrono::high_resolution_clock::now();
		}

		//reset timeout
		inline void ResetTimeOut(unsigned int time_out)
		{
			m_TimeOut = time_out;
		}

	private:
		//start time_poing
		std::chrono::time_point<std::chrono::high_resolution_clock> _start{ std::chrono::high_resolution_clock::now() };

		unsigned int m_TimeOut;
		CallBackFun m_CallBack;
	};

	typedef TimeWatcher<std::chrono::seconds> TimerSeconds;
	typedef TimeWatcher<std::chrono::milliseconds> TimerMillSeconds;
}