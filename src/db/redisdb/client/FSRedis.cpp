/*
* FSRedis.cpp
*
*  Created on: 2016-11-24
*
*  Author: yuanmj
*/

#include "FSRedis.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace db_redis;
using namespace common_cmmobj;
using namespace common_template;

#define _CHECK_CONNECT if(!m_pContext) return false;

FSRedisData::FSRedisData() :m_pReply(NULL)
{
}

FSRedisData::~FSRedisData()
{
	if (m_pReply)
	{
		freeReplyObject(m_pReply);
	}
}

redisReply* FSRedisData::getReply(int index)
{
	if (m_pReply->type == REDIS_REPLY_ARRAY)
	{
		return m_pReply->element[index];
	}
	else
	{
		return m_pReply;
	}
}

void FSRedisData::initReply(redisReply* reply)
{
	if (m_pReply)
	{
		freeReplyObject(m_pReply);
		m_pReply = NULL;
	}
	m_pReply = reply;
}

bool FSRedisData::checkType(int index)
{
	return !isNil(index) &&
		(getReply(index)->type == REDIS_REPLY_STRING ||
			getReply(index)->type == REDIS_REPLY_INTEGER ||
			getReply(index)->type == REDIS_REPLY_STATUS);
}

bool FSRedisData::isArray()
{
	return m_pReply->type == REDIS_REPLY_ARRAY;
}

bool FSRedisData::isNil(int index)
{
	return !m_pReply || m_pReply->type == REDIS_REPLY_NIL || m_pReply->type == REDIS_REPLY_ERROR;
}

int  FSRedisData::getInt(int index)
{
	if (!checkType(index))
	{
		return 0;
	}
	return atoi(getString());
}

float  FSRedisData::getFloat(int index)
{
	if (!checkType(index))
	{
		return 0;
	}
	return atof(getString());
}

const char*  FSRedisData::getString(int index)
{
	if (!checkType(index))
	{
		return 0;
	}
	return getReply(index)->str;
}

bool  FSRedisData::getBool(int index)
{
	if (!checkType(index))
	{
		return 0;
	}
	return strcasecmp("true", getReply(index)->str) == 0;
}

char*  FSRedisData::getBinary(int index)
{
	return getReply(index)->str;
}

size_t  FSRedisData::getLength(int index)
{
	return getReply(index)->len;
}

size_t FSRedisData::getArrayLength()
{
	if (m_pReply)
	{
		return m_pReply->elements;
	}
	return 0;
}

/*
* class FSRedis
*/

FSRedis::FSRedis() :m_pContext(nullptr)
{
}

FSRedis::~FSRedis()
{
	this->close();
}

bool FSRedis::connect(FSCacheConfig* conf)
{
	FSRedisConfig* config = (FSRedisConfig*)conf;
	if (!config)
	{
		return false;
	}

	m_pContext = redisConnectWithTimeout(config->host.c_str(), config->port, config->connect_timeout);
	if (NULL == m_pContext || m_pContext->err)
	{
		if (NULL != m_pContext)
		{
			LOG_ERROR("db_redis") << string_format("RDCONN|connect redis exception:%s\n ", m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
		}
		else
		{
			LOG_ERROR("db_redis") << "RDCONN|connect redis context fail\n";
		}

		return false;
	}
	else
	{
		LOG_INFO("db_redis") << string_format("RDCONN|connect redis(%s:%d) successful\n", config->host.c_str(), config->port);
	}

	if (REDIS_OK != redisSetTimeout(m_pContext, config->rw_timeout))
	{
		LOG_ERROR("db_redis") << string_format("RDCONN|connect redis(%s:%d) fail\n", config->host.c_str(), config->port);
		return false;
	}

	//密码为空不验证
	if (config->pwd.empty())
	{
		return true;
	}

	//密码验证
	redisReply* reply = (redisReply*)redisCommand(m_pContext, "AUTH %s", config->pwd.c_str());
	if (!checkReplay(reply))
	{
		return false;
	}

	if (REDIS_REPLY_ERROR == reply->type)
	{
		LOG_ERROR("db_redis") << string_format("RDCONN|connect redis(%s:%d:%s) pwd auth fail\n", config->host.c_str(), config->port, config->pwd.c_str());
		return false;
	}

	LOG_INFO("db_redis") << string_format("RDCONN|connect redis(%s:%d:%s) pwd auth successful\n", config->host.c_str(), config->port, config->pwd.c_str());
	return true;
}

bool FSRedis::close()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
		return true;
	}
	return false;
}

bool FSRedis::selectDB(int index)
{
	if (executeCommand("SELECT %d", index))
	{
		return true;
	}
	return false;
}

bool FSRedis::checkReplay(redisReply* replay)
{
	return replay != NULL;
}

int  FSRedis::pushStringToList(const char* list_name, const char* value)
{
	_CHECK_CONNECT
		int s32Ret = -1;
	const string strCmd = string_format("RPUSH %s %s", list_name, value);
	redisReply* reply = (redisReply*)redisCommand(m_pContext, strCmd.c_str());
	if (!checkReplay(reply))
	{
		return s32Ret;
	}

	if (REDIS_REPLY_INTEGER != reply->type)
	{
		LOG_ERROR("db_redis") << string_format("RDCMD|%s|execute rpush command fail\n", strCmd.c_str());
	}
	else
	{
		s32Ret = 0;
	}

	freeReplyObject(reply);
	return s32Ret;
}

bool FSRedis::delKey(const char* key)
{
	_CHECK_CONNECT
		const string strCmd = string_format("LPOP %s", key);
	redisReply* reply = (redisReply*)redisCommand(m_pContext, "%s", strCmd.c_str());
	if (!checkReplay(reply))
	{
		LOG_ERROR("db_redis") << string_format("RDCMD|%s|execute del command fail\n", strCmd.c_str());
		return false;
	}

	int ret = reply->integer;
	freeReplyObject(reply);
	return ret > 0;
}

bool FSRedis::getDataFromList(const char* list_name, size_t start, size_t end, FSCacheData* outdata)
{
	_CHECK_CONNECT
		const string strCmd = string_format("LRANGE %s %d -1", list_name, start);
	redisReply* reply = (redisReply*)redisCommand(m_pContext, strCmd.c_str());
	if (!checkReplay(reply))
	{
		LOG_ERROR("db_redis") << string_format("RDCMD|%s|execute lpush command fail\n", strCmd.c_str());
		return false;
	}

	((FSRedisData*)outdata)->initReply(reply);
	return true;
}

int FSRedis::getDBNumber()
{
	return 0;
}

bool FSRedis::executeCommand(const char* command, ...)
{
	_CHECK_CONNECT
		va_list ap;
	va_start(ap, command);
	bool ret = executeCommand(command, ap);
	va_end(ap);
	return ret;
}

bool FSRedis::executeCommand(const char* command, va_list ap)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, command, ap);
	if (!checkReplay(reply))
	{
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool FSRedis::setString(const char* key, const char* value, unsigned int expire)
{
	_CHECK_CONNECT
		redisReply* reply = NULL;
	if (expire != 0)
	{
		reply = (redisReply*)redisCommand(m_pContext, "SETEX %s %d %s", key, expire, value);
	}
	else
	{
		reply = (redisReply*)redisCommand(m_pContext, "SET %s %s", key, value);
	}

	if (!checkReplay(reply))
	{
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool FSRedis::setBinary(const char* key, unsigned char* data, size_t len)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "SET %s %b", key, data, len);
	if (!checkReplay(reply))
	{
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool FSRedis::setHashString(const char* hash_name, const char* key, const char* value)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "HSET %s %s %s", hash_name, key, value);
	if (!checkReplay(reply))
	{
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool FSRedis::setHashBinary(const char* hash_name, const char* key, unsigned char* data, size_t len)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "HSET %s %s %b", hash_name, key, data, len);
	if (!checkReplay(reply))
	{
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool  FSRedis::setStringToList(const char* list_name, int index, const char* value)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "LSET %s %d %s", list_name, index, value);
	if (!checkReplay(reply))
	{
		return false;
	}

	freeReplyObject(reply);
	return true;
}

bool  FSRedis::setBinaryToList(const char* list_name, int index, char* data, size_t len)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "LSET %s %d %b", list_name, index, data, len);
	if (!checkReplay(reply))
	{
		return false;
	}
	freeReplyObject(reply);
	return true;
}

int  FSRedis::pushBinaryToList(const char* list_name, char* binary, size_t len)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "RPUSH %s %b", list_name, binary, len);
	if (!checkReplay(reply))
	{
		return -1;
	}
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

void FSRedis::addSortedSet(const char* set_name, int score, const char* value)
{
	if (!m_pContext) return;
	redisReply* reply = (redisReply*)redisCommand(m_pContext, "ZADD %s %d %s", set_name, score, value);
	freeReplyObject(reply);
}

int  FSRedis::getSortByString(const char* set_name, const char* value)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "ZRANK %s %s", set_name, value);
	if (!checkReplay(reply))
	{
		return -1;
	}
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

bool FSRedis::getDataFromKey(const char* key, FSCacheData* outdata)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "GET %s", key);
	if (!checkReplay(reply))
	{
		return false;
	}

	((FSRedisData*)outdata)->initReply(reply);
	return true;
}

bool FSRedis::getDataFromHash(const char* hash_name, const char* key, FSCacheData* outdata)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "HGET %s %s", hash_name, key);
	if (!checkReplay(reply))
	{
		return false;
	}
	((FSRedisData*)outdata)->initReply(reply);
	return true;
}

int  FSRedis::getListLen(const char* list_name)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "LLEN %s", list_name);
	if (!checkReplay(reply))
	{
		return 0;
	}
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

bool  FSRedis::getDataFromSoctedSet(const char* set_name, size_t start, size_t end, FSCacheData* outdata) {
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "ZRANGE %s %d %d", set_name, start, end);
	if (!checkReplay(reply))
	{
		return false;
	}
	((FSRedisData*)outdata)->initReply(reply);
	return true;
}

bool FSRedis::shiftDataFromList(const char* list_name, FSCacheData* outdata)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "LPOP %s", list_name);
	if (!checkReplay(reply))
	{
		return false;
	}

	((FSRedisData*)outdata)->initReply(reply);
	return true;
}

bool FSRedis::popDataFromList(const char* list_name, FSCacheData* outdata)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "RPOP %s %s", list_name);
	if (!checkReplay(reply))
	{
		return false;
	}

	((FSRedisData*)outdata)->initReply(reply);
	return true;
}

int FSRedis::incrKey(const char* key)
{
	_CHECK_CONNECT
		redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCR %s", key);
	if (!checkReplay(reply)) return false;
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}