#include "Redis.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace REDIS_DB;
using namespace common_cmmobj;
using namespace common_template;

void Redis::Init(const string& strRdIp, const string& strRdPwd, int s32RdPort, int s32RdConnTimeout, int s32RdRWTimeout)
{
	conf.db_index = 0;
	conf.host = strRdIp;
	conf.pwd = strRdPwd;
	conf.port = s32RdPort;
	conf.connect_timeout.tv_sec = s32RdConnTimeout;
	conf.connect_timeout.tv_usec = 0;

	conf.rw_timeout.tv_sec = s32RdRWTimeout;
	conf.rw_timeout.tv_usec = 0;
}

Redis::Redis()
	: m_pool_index(0)
{
}

Redis::~Redis()
{
	close();
}

//connect redis server
bool Redis::connect()
{
	return db.connect(&conf);
}

//close connect to redis server
void Redis::close()
{
	db.close();
}

//reconnect to redis
bool Redis::reConnect()
{
	close();
	return connect();
}

//push key-value list
int Redis::rpush_list(std::string key, const char* pData)
{
	int s32Ret = db.pushStringToList(key.c_str(), pData);
	if (0 <= s32Ret)
	{
		return s32Ret;
	}

	if (reConnect())
	{
		LOG_WARN("db_redis") << string_format("rpush key(%s) and data(%s) reconnect redis successful\n", key.c_str(), pData);
		s32Ret = db.pushStringToList(key.c_str(), pData);
	}
	else
	{
		LOG_WARN("db_redis") << string_format("rpush key(%s) and data(%s) reconnect redis fail\n", key.c_str(), pData);
	}
	return s32Ret;
}

//del list
void Redis::del_list(std::string key)
{
	bool bRet = db.shiftDataFromList(key.c_str(), &data);
	if (bRet)
	{
		LOG_DEBUG("db_redis") << string_format("del list key(%s) successful\n", key.data());
		return;
	}

	if (reConnect())
	{
		LOG_WARN("db_redis") << string_format("del list key(%s) reconnect redis successful\n", key.c_str());
		bRet = db.delKey(key.c_str());
	}
	else
	{
		LOG_WARN("db_redis") << string_format("del list key(%s) reconnect redis fail\n", key.c_str());
	}
}

//read key-value list
void Redis::lrange_list(std::string key, vector<std::string>& datalist)
{
	bool bRet = db.getDataFromList(key.c_str(), 0, -1, &data);
	if (!bRet)
	{
		if (reConnect())
		{
			LOG_WARN("db_redis") << string_format("lrange_list key(%s) reconnect redis successful\n", key.c_str());
			db.getDataFromList(key.c_str(), 0, -1, &data);
		}
		else
		{
			LOG_WARN("db_redis") << string_format("lrange_list key(%s) reconnect redis fail\n", key.c_str());
			return;
		}
	}

	int nArrayLen = data.getArrayLength();
	for (int i = 0; i < nArrayLen; i++)
	{
		std::string strData(data.getString(i), data.getLength(i));
		datalist.push_back(strData);
	}
}

void Redis::setPoolIndex(int index)
{
	m_pool_index = index;
}

int Redis::getPoolIndex()
{
	return m_pool_index;
}