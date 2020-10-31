#include "SqliteDbMng.h"
#include "json/JsonCpp.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"

using namespace std;
using namespace SQLITE_DB;
using namespace common_cmmobj;
using namespace common_template;
using namespace IF_SQLITE;

const string strDbFilePath = string("./mysqlite.db");

//API
extern "C" bool SqliteInit(const VecTupleApiType& vecCfg)
{
    return SCSqliteDbMng.InitDb(vecCfg);
}

extern "C" bool SqliteExcecuteJson(const string& strSql,const CJsonCpp& jData)
{
    return SCSqliteDbMng.ExcecuteJson(strSql,jData);
}

extern "C" void SqliteQuery(const string& strSql, rapidjson::Document& doc)
{
    SCSqliteDbMng.Query(strSql,doc);
}

extern "C" bool SqliteExcecute(const std::string& strSql)
{
    return SCSqliteDbMng.Excecute(strSql);
}

//保存数据到数据库
bool CSqliteDbMng::ExcecuteJson(const string& strSql,const CJsonCpp& jcp)
{
    bool bRet = smartDB.ExcecuteJson(strSql,jcp.GetString());
    return bRet;
}

//从数据库中删除数据
bool CSqliteDbMng::Excecute(const string& strSql)
{
    bool bRet = smartDB.Excecute(strSql);
    return bRet;
}

//从数据库读数据
void CSqliteDbMng::Query(const string& strSql,rapidjson::Document& doc)
{
    smartDB.Query(strSql, doc);
}

//初始化数据库
bool CSqliteDbMng::InitDb(const IF_SQLITE::VecTupleApiType& vecCfg)
{
    //创建数据库
    smartDB.Open(strDbFilePath);

    //创建数据库表
    bool bRet = false;
    for(auto val : vecCfg)
    {
        string strCreateSql,strSelectSql,strInsertSql;
        CJsonCppShrPtr jDbCfg;
        tie(strCreateSql,strSelectSql,strInsertSql,jDbCfg) = val;
    
        bRet = smartDB.Excecute(strCreateSql);
        if(!bRet)
        {
             continue;
        }

        rapidjson::Document doc;
        smartDB.Query(strSelectSql, doc);
        if(0 < doc.Size())
        {
            continue;
        }

        if(strInsertSql.empty())
        {
            continue;
        }

        bRet = smartDB.ExcecuteJson(strInsertSql,jDbCfg->GetString());
    }

    return bRet;
}