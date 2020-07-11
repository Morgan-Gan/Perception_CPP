#pragma once
#include <chrono>
#include <types.h>

using namespace std;
using namespace std::chrono;

namespace common_cmmobj
{
	class Timer
	{
	public:
		Timer() : m_begin(high_resolution_clock::now()) {}
		void reset() { m_begin = high_resolution_clock::now(); }

		//Ĭ���������
		int64_t elapsed() const
		{
			return duration_cast<chrono::milliseconds>(high_resolution_clock::now() - m_begin).count();
		}

		//Ĭ�������
		double elapsed_second() const
		{
			return duration_cast<duration<double>>(high_resolution_clock::now() - m_begin).count();
		}

		//΢��
		int64_t elapsed_micro() const
		{
			return duration_cast<chrono::microseconds>(high_resolution_clock::now() - m_begin).count();
		}

		//����
		int64_t elapsed_nano() const
		{
			return duration_cast<chrono::nanoseconds>(high_resolution_clock::now() - m_begin).count();
		}

		//��
		int64_t elapsed_seconds() const
		{
			return duration_cast<chrono::seconds>(high_resolution_clock::now() - m_begin).count();
		}

		//��
		int64_t elapsed_minutes() const
		{
			return duration_cast<chrono::minutes>(high_resolution_clock::now() - m_begin).count();
		}

		//ʱ
		int64_t elapsed_hours() const
		{
			return duration_cast<chrono::hours>(high_resolution_clock::now() - m_begin).count();
		}

	private:
		time_point<high_resolution_clock> m_begin;
	};
}