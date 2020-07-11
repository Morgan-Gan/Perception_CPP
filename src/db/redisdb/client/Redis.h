#ifndef _REDIS_H_
#define _REDIS_H_

#include <string>
#include "CommFun.h"
#include "FSRedis.h"

namespace db_redis
{
	class Redis
	{
	public:
		Redis();
		~Redis();

	public:
		void Init(const std::string& RdIp, const std::string& RdPwd, int s32RdPort, int s32RdConnTimeout, int s32RdRWTimeout);
		bool connect();
		int	 rpush_list(std::string key, const char* strData);
		void del_list(std::string key);
		void lrange_list(std::string key, std::vector<std::string>& datalist);
		void setPoolIndex(int index);
		int  getPoolIndex();

	private:
		void close();
		bool reConnect();

	private:
		FSRedisData data;
		FSRedisConfig conf;
		FSRedis db;

		int  m_pool_index;
	};
}
#endif  //_REDIS_H_
