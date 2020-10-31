#pragma once
#include <string>
#include <vector>

namespace IF_REDIS
{
    extern "C" bool RedisInit(const std::string&, const std::string&, int, int, int, int, int);
    extern "C" bool RedisSaveData(const std::string&, const std::string&);
    extern "C" void RedisReadData(const std::string&, std::vector<std::string>& vecData);
    extern "C" void RedisDelData(const std::string&);
}