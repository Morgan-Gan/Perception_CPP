#ifndef __LIBMUTEX_H__
#define __LIBMUTEX_H__

#if defined(__linux)
#include <pthread.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace common_cmmobj
{
#if defined(__linux) || defined(_WIN32)
#pragma pack(push, 1)
#endif
	typedef struct
	{
#if defined(__linux)
		pthread_mutex_t key1;
		pthread_mutexattr_t key2;
#elif defined(_WIN32)
		HANDLE m_mutex;
#endif
	} SMutexData;

#if defined(__linux) || defined(_WIN32)
#pragma pack(pop)
#endif
	class CMutex
	{
	public:
		CMutex();
		~CMutex();
		void Lock();
		void Unlock();
	private:
		void MutexInit(SMutexData *mutex);
		void MutexLock(SMutexData *mutex);
		void MutexUnlock(SMutexData *mutex);
		void MutexDestroy(SMutexData *mutex);
	private:
		SMutexData mMutexdata;
	};

	class CLock
	{
		CMutex *mMutex;
	public:
		CLock(CMutex *mutex);
		~CLock();
	};
}
#endif


