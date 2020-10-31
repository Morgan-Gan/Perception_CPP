#pragma once
#include <string>
#include <tuple>
#include <vector>
#include "comm/CommDefine.h"
#include "json/JsonCpp.h"

namespace IF_SQLITE
{   	
    using CJsonCppShrPtr = std::shared_ptr<common_cmmobj::CJsonCpp>;
    using TupleApiType = std::tuple<std::string,std::string,std::string,CJsonCppShrPtr>;
	using VecTupleApiType = std::vector<TupleApiType>;
    extern "C" bool SqliteInit(const VecTupleApiType&);
    extern "C" bool SqliteExcecuteJson(const std::string& strSql,const common_cmmobj::CJsonCpp& jData);
    extern "C" void SqliteQuery(const std::string& strSql, rapidjson::Document& doc);
    extern "C" bool SqliteExcecute(const std::string& strSql);
}