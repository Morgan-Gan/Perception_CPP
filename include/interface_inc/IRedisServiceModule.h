#pragma once
#include <string>
#include <vector>

namespace IF_REDIS
{
    extern "C" bool InitRedis(const std::string&, const std::string&, int, int, int, int, int);
    extern "C" bool SaveDataToRedis(const std::string&, const std::string&);
    extern "C" void GetDataFromRedis(const std::string&, std::vector<std::string>& vecData);
    extern "C" void DelDataFromRedis(const std::string&);
}