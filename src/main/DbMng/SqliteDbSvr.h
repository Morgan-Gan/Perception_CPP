#pragma once
#include "comm/CommDefine.h"
#include "json/JsonCpp.h"

namespace MAIN_MNG
{
    class SqliteDbSvr
    {
    public:
        SqliteDbSvr() = default;
        ~SqliteDbSvr() = default;

        bool SysCfgDBInit();
		void SysCfgDBOpt(bool bInit = true);

    private:
        void DbTransferCfgTableDOpt(std::tuple<std::string,bool>&& tpParam);
		void AlgCfgTableDbOpt(bool bInit = true);
		void TaskCfgTableDbOpt(bool bInit = true);
		void AppCfgTableDbOpt(bool bInit = true);
		void ReportTableOpt();
		bool SqliteExcecuteJson(const std::string& strSql,const common_cmmobj::CJsonCpp& jData);
		void SqliteQuery(const std::string& strSql,rapidjson::Document& jData);
		bool SqliteExcecute(const std::string& strSql);
    };    
}