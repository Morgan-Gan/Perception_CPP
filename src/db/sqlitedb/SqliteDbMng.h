#pragma once
#include <string>
#include <vector>
#include "SmartDB.h"
#include "comm/Singleton.h"
#include "ISqliteServiceModule.h"

namespace SQLITE_DB
{
    class CSqliteDbMng : public common_template::CSingleton<CSqliteDbMng>
	{
		friend class common_template::CSingleton<CSqliteDbMng>;
    public:
        bool InitDb(const IF_SQLITE::VecTupleApiType& vecCfg);
        bool ExcecuteJson(const std::string& strSql,const common_cmmobj::CJsonCpp& jData);
        void Query(const std::string& strSql,rapidjson::Document& doc);
        bool Excecute(const std::string& strSql);

    protected:
        CSqliteDbMng() = default;
        ~CSqliteDbMng(){ smartDB.Close();}
    private:
         CSmartDB smartDB;
    };
    #define SCSqliteDbMng (common_template::CSingleton<CSqliteDbMng>::GetInstance())
}