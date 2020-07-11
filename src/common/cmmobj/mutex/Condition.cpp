#include "Condition.h"

using namespace common_cmmobj;

CCondition::CCondition(MutexLock &mutex)
	:mutex_(mutex)
{
	if (pthread_cond_init(&cond_, NULL))
	{
		LOG_FATAL("systerm") << "pthread_cond_init Fail!\n";
	}
}

CCondition::~CCondition()
{
	if (pthread_cond_destroy(&cond_))
	{
		LOG_FATAL("systerm") << "pthread_cond_destroy Fail!\n";
	}
}

void CCondition::wait()
{
	assert(mutex_.isLocking());
	if (pthread_cond_wait(&cond_, mutex_.getMutexPtr()))
	{
		LOG_FATAL("systerm") << "pthread_cond_wait Fail!\n";
	}
	mutex_.restoreMutexStatus();
}

void CCondition::notify()
{
	if (pthread_cond_signal(&cond_))
	{
		LOG_FATAL("systerm") << "pthread_cond_signal Fail!\n";
	}
}

void CCondition::notifyAll()
{
	if (pthread_cond_broadcast(&cond_))
	{
		LOG_FATAL("systerm") << "pthread_cond_broadcast Fail!\n";
	}
}

