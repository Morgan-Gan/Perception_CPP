#pragma once
#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "log4cxx/Loging.h"
#include <assert.h>

namespace common_cmmobj
{
	class MutexLock : boost::noncopyable
	{
		friend class CCondition;

	public:
		MutexLock()
			:isLocking_(false)
		{
			if (pthread_mutex_init(&mutex_, NULL))
			{
				LOG_FATAL("systerm") << "pthread_mutex_init Fail!\n";
			}
		}

		~MutexLock()
		{
			assert(!isLocking());
			if (pthread_mutex_destroy(&mutex_))
			{
				LOG_FATAL("systerm") << "pthread_mutex_init Fail!\n";
			}
		}

		void lock()
		{
			if (pthread_mutex_lock(&mutex_))
			{
				LOG_FATAL("systerm") << "pthread_mutex_init Fail!\n";
			}
			isLocking_ = true;
		}

		void unlock()
		{
			isLocking_ = false;
			if (pthread_mutex_unlock(&mutex_))
			{
				LOG_FATAL("systerm") << "pthread_mutex_init Fail!\n";
			}
		}

		bool isLocking() const { return isLocking_; }
		pthread_mutex_t *getMutexPtr()
		{
			return &mutex_;
		}

		void restoreMutexStatus()
		{
			isLocking_ = true;
		}

	private:
		pthread_mutex_t mutex_;
		bool isLocking_;
	};

	class MutexLockGuard : boost::noncopyable
	{
	public:
		MutexLockGuard(MutexLock &mutex) :mutex_(mutex)
		{
			mutex_.lock();
		}
		~MutexLockGuard()
		{
			mutex_.unlock();
		}

	private:
		MutexLock &mutex_;
	};

	class CCondition : boost::noncopyable
	{
	public:
		CCondition(MutexLock &mutex);
		~CCondition();

		void wait();
		void notify();
		void notifyAll();

	private:
		pthread_cond_t cond_;
		MutexLock &mutex_;
	};
}