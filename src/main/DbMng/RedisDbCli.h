#pragma once
#include "comm/CommDefine.h"

namespace MAIN_MNG
{
    class CRedisDbCli
    {
    public:
        CRedisDbCli() = default;
        ~CRedisDbCli() = default;

        bool RedisInit();

	private:	
		bool RedisSaveData(const std::string& strKey, const std::string& strData);
		void RedisReadData(const std::string& strKey, std::vector<std::string>& vecData);
		void RedisDelData(const std::string& strKey);
    };
} // namespace name
