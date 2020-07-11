#include "RedisMng.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace db_redis;
using namespace common_cmmobj;
using namespace common_template;

const int s32MaxDefaultConnections = 20;
const int s32InitDefaultConnections = 10;
const int nDefaultRedisPort = 47393;
const string strDefaultRedisIp = "127.0.0.1";
const string strDefaultRedisPwd = "";
const int s32RedisDefaultConnTimeOut = 2;
const int s32RedisDefaultRWTimeout = 5;

extern "C" bool IF_REDIS::InitRedis(const string& strRdIp, const string& strRdPwd, int s32RdPort, int s32RdConnTimeout, int s32RdRWTimeout, int maxConnections, int initConnections)
{
	return SRedisMng.Init(strRdIp, strRdPwd, s32RdPort, s32RdConnTimeout, s32RdRWTimeout, maxConnections, initConnections);
}

extern "C" bool IF_REDIS::SaveDataToRedis(const std::string& strKey, const std::string& strData)
{
	bool bLongConn = true;
	Redis* pRedis = SRedisMng.Get(bLongConn);
	if (!pRedis)
	{
		return false;
	}

	int s32Ret = pRedis->rpush_list(strKey, strData.c_str());
	SRedisMng.Put(pRedis, bLongConn);

	return (0 == s32Ret) ? true : false;
}

extern "C" void IF_REDIS::GetDataFromRedis(const std::string& strKey, std::vector<std::string>& vecData)
{
	bool bLongConn = true;
	Redis* pRedis = SRedisMng.Get(bLongConn);
	if (!pRedis)
	{
		return;
	}

	pRedis->lrange_list(strKey, vecData);
	SRedisMng.Put(pRedis, bLongConn);
}

extern "C" void IF_REDIS::DelDataFromRedis(const std::string& strKey)
{
	bool bLongConn = true;
	Redis* pRedis = SRedisMng.Get(bLongConn);
	if (!pRedis)
	{
		return;
	}

	pRedis->del_list(strKey);
	SRedisMng.Put(pRedis, bLongConn);
}

RedisMng::RedisMng() :
	m_maxConnections(0),
	m_currentConnections(0)
{
}

RedisMng::~RedisMng()
{
	delete[]m_connections;
	delete[]m_is_free;
}

Redis * RedisMng::newConnection(bool& bLongConn)
{
	Redis *redis = new Redis();
	redis->Init(m_strRedisIp, m_strRedisPwd, m_s32RedisPort, m_s32RedisConnectTimeout, m_s32RedisRWTimeout);
	if (!redis->connect())
	{
		delete redis;
		redis = NULL;
		return NULL;
	}

	//only creat short connect when the current connections > maxconnections
	if (m_currentConnections >= m_maxConnections)
	{
		LOG_WARN("db_redis") << string_format("RedisMng: connection exceed max connection count %d��", m_maxConnections);
		bLongConn = false;
		return redis;
	}

	m_connections[m_currentConnections] = redis;
	m_is_free[m_currentConnections] = true;
	redis->setPoolIndex(m_currentConnections);
	m_currentConnections++;
	bLongConn = true;

	return redis;
}

bool RedisMng::Init(const string& strRdIp, const string& strRdPwd, int s32RdPort, int s32RdConnTimeout, int s32RdRWTimeout, int maxConnections, int initConnections)
{
	//������������
	m_strRedisIp = (strRdIp.empty()) ? strDefaultRedisIp : strRdIp;
	m_strRedisPwd = (strRdPwd.empty()) ? strDefaultRedisPwd : strRdPwd;
	m_s32RedisPort = (0 >= s32RdPort) ? nDefaultRedisPort : s32RdPort;
	m_s32RedisConnectTimeout = (0 >= s32RdConnTimeout) ? s32RedisDefaultConnTimeOut : s32RdConnTimeout;
	m_s32RedisRWTimeout = (0 >= s32RdRWTimeout) ? s32RedisDefaultRWTimeout : s32RdRWTimeout;

	if (initConnections > maxConnections)
	{
		LOG_WARN("db_redis") << string_format("redis maxConnections(%d) < initConnections(%d)\n", maxConnections, initConnections);
		maxConnections = initConnections;
	}
	m_maxConnections = 0 < maxConnections ? maxConnections : s32MaxDefaultConnections;

	//creat Redis obj array
	m_connections = new Redis*[m_maxConnections]();
	m_is_free = new bool[m_maxConnections]();
	for (int i = 0; i < m_maxConnections; i++)
	{
		m_connections[i] = nullptr;
		m_is_free[i] = true;
	}
	return true;
}

Redis * RedisMng::Get(bool& bLongConn)
{
	Redis *redis = NULL;

	CLock lock(&m_mutex_of_connection);
	for (int i = 0; i < m_currentConnections; i++)
	{
		if (m_is_free[i])
		{
			m_is_free[i] = false;
			redis = m_connections[i];
			bLongConn = true;
			return redis;
		}
	}

	if (!redis)
	{
		redis = newConnection(bLongConn);
	}

	return redis;
}

void RedisMng::Put(Redis * redis, bool bLongConn)
{
	if (!redis)
	{
		return;
	}

	//release short connect and redis
	if (!bLongConn)
	{
		delete redis;
		redis = NULL;
		return;
	}

	//put redis to pool
	CLock lock(&m_mutex_of_connection);
	int s32ConnIndex = redis->getPoolIndex();
	m_is_free[s32ConnIndex] = true;
}