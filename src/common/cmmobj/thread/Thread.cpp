#include "Thread.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;

CThread::CThread(Func Cb, const int s32SecCycle, const int s32mSecCycle, string strThreadName,int s32CpuId) :
	m_Cb(Cb),
	m_s32SecCycle(s32SecCycle),
	m_s32mSecCycle(s32mSecCycle),
	m_strThreadName(strThreadName),
	m_s32CpuId(s32CpuId)
{
	Init();
}

CThread::~CThread()
{
	Unit();
}

bool CThread::StartThread()
{
	return CreatThread();
}

void CThread::Init()
{
	PthreadMutexCondInit(m_tThreadInfo);
	StartThread();
}

void CThread::Unit()
{
	PthreadMutexCondUninit(m_tThreadInfo);
}

bool CThread::CreatThread()
{
	//create thread
	int s32Ret = 0;
	if ((s32Ret = pthread_create(&m_tThreadInfo.i_id, NULL, ThreadLoop, this)) != 0)
	{
		LOG_FATAL("systerm") << string_format("Create Thread(%s) Fail!\n", m_strThreadName.c_str());
	}
	else
	{
		LOG_INFO("systerm") << string_format("Create Thread(%s) Successful!\n", m_strThreadName.c_str());
	}

	//bind a cpu
	int s32CpuCount = GetCpuCount();
	if (0 <= m_s32CpuId && m_s32CpuId <= (s32CpuCount - 1))
	{
		cpu_set_t cpu_info;
		CPU_ZERO(&cpu_info);
		CPU_SET(m_s32CpuId, &cpu_info);
		if (0 != pthread_setaffinity_np(m_tThreadInfo.i_id, sizeof(cpu_set_t), &cpu_info))
		{
			LOG_ERROR("systerm") << "set affinity_np fail";
		}
	}

	return (0 == s32Ret) ? true : false;
}

void * CThread::ThreadLoop(void * ptr)
{
	CThread* pThread = static_cast<CThread*>(ptr);
	TPthreadMutexCondInfo* pThreadInfo = &(pThread->m_tThreadInfo);
	while (true)
	{
		struct timespec tv = { 0 };
		pThread->GetTv(tv, pThread->m_s32SecCycle, pThread->m_s32mSecCycle);

		pthread_mutex_lock(&pThreadInfo->i_mutex);
		if (0 == pthread_cond_timedwait(&pThreadInfo->i_cv, &pThreadInfo->i_mutex, (const timespec *)&tv))
		{
			break;
		}

		pThread->m_Cb();

		pthread_mutex_unlock(&pThreadInfo->i_mutex);
	}

	pthread_exit(NULL);
	return NULL;
}