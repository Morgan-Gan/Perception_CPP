#include "LibMutex.h"

using namespace common_cmmobj;
CMutex::CMutex()
{
	MutexInit(&mMutexdata);
}

CMutex::~CMutex()
{
	MutexDestroy(&mMutexdata);
}

void CMutex::Lock()
{
	MutexLock(&mMutexdata);
}

void CMutex::Unlock()
{
	MutexUnlock(&mMutexdata);
}

void CMutex::MutexInit(SMutexData *mutex)
{
#if defined(__linux)
	pthread_mutexattr_init(&mutex->key2);
	pthread_mutexattr_settype(&mutex->key2, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
	pthread_mutex_init(&mutex->key1, &mutex->key2);
#elif defined(_WIN32)
	mutex->m_mutex = ::CreateMutex(NULL, FALSE, NULL);
#endif
}

void CMutex::MutexLock(SMutexData *mutex)
{
#if defined(__linux)
	pthread_mutex_lock(&mutex->key1);
#elif defined(_WIN32)
	DWORD d = WaitForSingleObject(mutex->m_mutex, INFINITE);
#endif
}

void CMutex::MutexUnlock(SMutexData *mutex)
{
#if defined(__linux)
	pthread_mutex_unlock(&mutex->key1);
#elif defined(_WIN32)
	::ReleaseMutex(mutex->m_mutex);
#endif
}

void CMutex::MutexDestroy(SMutexData *mutex)
{
#if defined(__linux)
	pthread_mutex_destroy(&mutex->key1);
#elif defined(_WIN32)
	::CloseHandle(mutex->m_mutex);
#endif
}

CLock::CLock(CMutex *mutex)
{
	mMutex = mutex;
	mMutex->Lock();
}

CLock::~CLock()
{
	mMutex->Unlock();
}
