
/*
* FSRedis.h
*
*  Created on: 2014-5-8
*      Author: yuanmj
*/

#ifndef FSREDIS_H_
#define FSREDIS_H_

#include <string>
#include <hiredis/hiredis.h>
#include "FSCacheDB.h"
#include "comm/CommFun.h"

namespace REDIS_DB
{
	class FSRedisConfig : public FSCacheConfig
	{
	public:
		std::string host;
		std::string pwd;
		int port;
		int db_index;
		struct timeval connect_timeout;
		struct timeval rw_timeout;
	};

	class FSRedis : public FSCacheDB
	{
	public:
		FSRedis();
		virtual ~FSRedis();

	private:
		redisContext* m_pContext;

	public:
		virtual bool setString(const char* key, const char* value, unsigned int expire = 0);
		virtual bool setBinary(const char* key, unsigned char* data, size_t len);

		virtual bool setHashString(const char* hash_name, const char* key, const char* value);
		virtual bool setHashBinary(const char* hash_name, const char* key, unsigned char* data, size_t len);

		virtual bool setStringToList(const char* list_name, int index, const char* value);
		virtual bool setBinaryToList(const char* list_name, int index, char* data, size_t len);

		virtual int  pushStringToList(const char* list_name, const char* value);
		virtual int  pushBinaryToList(const char* list_name, char* binary, size_t len);

		virtual void addSortedSet(const char* set_name, int score, const char* value);

	public:
		virtual int  getSortByString(const char* set_name, const char* value);
		virtual int  getListLen(const char* list_name);
		virtual bool getDataFromList(const char* list_name, size_t start, size_t end, FSCacheData* outdata);
		virtual bool getDataFromSoctedSet(const char* set_name, size_t start, size_t end, FSCacheData* outdata);
		virtual bool getDataFromKey(const char* key, FSCacheData* outdata);
		virtual bool getDataFromHash(const char* hash_name, const char* key, FSCacheData* outdata);
		virtual bool shiftDataFromList(const char* list_name, FSCacheData* outdata);
		virtual bool popDataFromList(const char* list_name, FSCacheData* outdata);

	public:
		virtual int incrKey(const char* key);
		virtual bool delKey(const char* key);

	public:
		virtual bool selectDB(int index);
		virtual int getDBNumber();

	public:
		bool connect(FSCacheConfig* conf);
		bool close();

	public:
		bool checkReplay(redisReply*);
		bool executeCommand(const char* command, ...);
		bool executeCommand(const char* command, va_list ap);
	};

	class FSRedisData : public FSCacheData
	{
	private:
		redisReply* m_pReply;
		friend class FSRedis;

		void initReply(redisReply*);

	private:
		bool checkType(int index);

	public:
		FSRedisData();
		~FSRedisData();

	private:
		redisReply* getReply(int index = 0);

	public:
		virtual bool isNil(int index = 0);
		virtual int getInt(int index = 0);
		virtual float getFloat(int index = 0);
		virtual const char* getString(int index = 0);
		virtual bool getBool(int index = 0);
		virtual char* getBinary(int index = 0);
		virtual size_t getLength(int index = 0);
		virtual bool isArray();
		virtual size_t getArrayLength();
	};
}
#endif /* FSREDIS_H_ */

