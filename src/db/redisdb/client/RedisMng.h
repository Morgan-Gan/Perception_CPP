#ifndef _REDIS_MANAGER_H_
#define _REDIS_MANAGER_H_

#include <string>
#include "Redis.h"
#include "mutex/LibMutex.h"
#include "comm/Singleton.h"
#include "IRedisServiceModule.h"

namespace db_redis
{
	class RedisMng : public common_template::CSingleton<RedisMng>
	{
		friend class common_template::CSingleton<RedisMng>;
		using MutexType = common_cmmobj::CMutex;
	public:
		bool Init(const std::string& strRdIp, const std::string& strRdPwd,int s32RdPort, int s32RdConnTimeout, int s32RdRWTimeout, int maxConnections, int initConnections);
		Redis* Get(bool& bLongConn);
		void Put(Redis* redis, bool bLongConn);

	protected:
		RedisMng();
		~RedisMng();

	private:
		Redis* newConnection(bool& bLongConn);

	private:
		MutexType    m_mutex_of_connection;
		int			 m_maxConnections;
		int			 m_currentConnections;
		std::string  m_strRedisIp;
		std::string m_strRedisPwd;
		int          m_s32RedisPort;
		int          m_s32RedisConnectTimeout;
		int          m_s32RedisRWTimeout;

		Redis**     m_connections;
		bool*       m_is_free;
	};

#define  SRedisMng (common_template::CSingleton<RedisMng>::GetInstance())
}
#endif  //_REDIS_MANAGER_H_


