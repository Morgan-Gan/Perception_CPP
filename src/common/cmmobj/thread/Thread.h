#pragma once
#include <functional>
#include "comm/CommFun.h"
#include <pthread.h>
#include "mutex/LibMutex.h"

namespace common_cmmobj
{
	class CThread
	{
		using Func = std::function<void()>;

#pragma pack(push, 1)
		typedef struct MutexCondInfo
		{
			pthread_condattr_t cattr;
			pthread_t i_id;
			pthread_mutex_t i_mutex;
			pthread_cond_t i_cv;
			void* i_sigevent;
		}TPthreadMutexCondInfo;
#pragma pack(pop)


	public:
		CThread(Func Cb, const int s32SecCycle, const int s32mSecCycle, const std::string& strThreadFlag,const int& s32CpuId);

		template<class Fc,class ...Args>
		CThread(const int& s32SecCycle, const int& s32mSecCycle, const std::string& strThreadFlag,const int& s32CpuId,Fc Cb, Args && ...args) :
				m_s32SecCycle(s32SecCycle),
				m_s32mSecCycle(s32mSecCycle),
				m_strThreadFlag(strThreadFlag),
				m_s32CpuId(s32CpuId),
				m_Cb(std::bind(Cb,std::forward<Args>(args)...))
		{
			Init();
		}

		~CThread();

	private:
		void Init();
		void Unit();
		bool StartThread();
		bool CreatThread();
		static void* ThreadLoop(void* ptr);

	private:
		inline bool PthreadMutexCondInit(TPthreadMutexCondInfo& mcond)
		{
			int nRet = pthread_condattr_init(&(mcond.cattr));
			if (0 != nRet)
			{
				return false;
			}

			nRet = pthread_mutex_init(&(mcond.i_mutex), NULL);
			nRet = pthread_condattr_setclock(&(mcond.cattr), CLOCK_MONOTONIC);
			nRet = pthread_cond_init(&(mcond.i_cv), &(mcond.cattr));
			mcond.i_sigevent = NULL;

			return true;
		}

		inline bool PthreadMutexCondUninit(TPthreadMutexCondInfo& mcond)
		{
			pthread_condattr_destroy(&(mcond.cattr));
			pthread_mutex_lock(&(mcond.i_mutex));
			pthread_cond_signal(&(mcond.i_cv));
			pthread_mutex_unlock(&(mcond.i_mutex));
			pthread_join(mcond.i_id, NULL);
			pthread_mutex_destroy(&(mcond.i_mutex));
			pthread_cond_destroy(&(mcond.i_cv));
			return true;
		}

		inline void GetTv(struct timespec& tv, const int s32Sec, const int s32mSec)
		{
			clock_gettime(CLOCK_MONOTONIC, &tv);
			tv.tv_sec += s32Sec;
			tv.tv_nsec += (s32mSec * 1000 * 1000);
		}

		inline int GetCpuCount()
		{
			return (int)sysconf(_SC_NPROCESSORS_ONLN);
		}

	private:
		int m_s32SecCycle;
		const int m_s32mSecCycle;
		Func const m_Cb;
		TPthreadMutexCondInfo m_tThreadInfo;
		std::string const m_strThreadFlag;

		int m_s32CpuId;
	};
}