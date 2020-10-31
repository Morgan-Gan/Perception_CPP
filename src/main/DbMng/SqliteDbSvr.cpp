#include "SqliteDbSvr.h"
#include "ISqliteServiceModule.h"
#include "MsgBusMng/MsgBusMng.h"
#include "CfgMng/CfgMng.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include "comm/FuncInitLst.h"

using namespace std;
using namespace MAIN_MNG;
using namespace common_cmmobj;
using namespace common_template;

auto ParseSubElmFun = [](string& strResult,const string& strSubName,const Json& elm)
{
	Json subElm = (strSubName.empty()) ? elm : elm[strSubName];
	for (auto it = subElm.begin();it != subElm.end();++it)
	{
		string&& strAttribute = string(it.key());
		strResult += strAttribute;
		strResult += string(":");
		strResult += string(it.value());
		strResult += string("|");
	}
};

auto DbExeInitFun = [](string&& strCreateSql,string&& strSelectSql,string&& strInsertSql,IF_SQLITE::CJsonCppShrPtr&& jcp)
{
	IF_SQLITE::VecTupleApiType vecTupleParam;
	IF_SQLITE::TupleApiType api = make_tuple<string,string,string,IF_SQLITE::CJsonCppShrPtr>(move(strCreateSql),move(strSelectSql),move(strInsertSql),move(jcp));
	vecTupleParam.emplace_back(api);
	IF_SQLITE::SqliteInit(vecTupleParam);
};

void SqliteDbSvr::AppCfgTableDbOpt(bool bInit)
{
	vector<Json> vecCfg;
	bInit ? SCCfgMng.GetExFactorySysCfg(strAppNode,vecCfg) : SCCfgMng.GetSysCfgJsonObj(strAppNode,vecCfg);
	if(vecCfg.empty())
	{
		return;
	}

	string&& strTableName = string(strAppNode);
	string&& strCreatSql = string_format("create table if not exists %s(id INTEGER PRIMARY KEY autoincrement,node TEXT,type TEXT,alg_idx TEXT,data_src TEXT,task_lst TEXT,start_time TEXT,end_time TEXT,idx TEXT);",strTableName.c_str());
	string&& strSelectSql = string_format("select * from %s;",strTableName.c_str());
	string&& strInsertSql = string_format("INSERT INTO %s(id,node,type,alg_idx,data_src,task_lst,start_time,end_time,idx) VALUES(?,?,?,?,?,?,?,?,?);",strTableName.c_str());
	
	IF_SQLITE::CJsonCppShrPtr jcp(new CJsonCpp());
	jcp->StartArray();
	for (auto val : vecCfg)
	{
		for(auto it = val.begin();it != val.end();it++)
		{
			string&& strSubNode = string(it.key());
			for(auto elm : it.value())
			{
				jcp->StartObject();
				jcp->WriteJson(strAttriId.c_str(),nullptr);
				jcp->WriteJson(strAttriNode.c_str(), strSubNode.c_str());
				jcp->WriteJson(strAttriType.c_str(),string(elm[strAttriType]).c_str());
				jcp->WriteJson(strAttriAlgIdx.c_str(),string(elm[strAttriAlgIdx]).c_str());

				string strDataSrc("");
				for(auto subElm : elm[strAttriDataSrc])
				{
					ParseSubElmFun(strDataSrc,"",subElm);
					strDataSrc += string("&");
				}
				jcp->WriteJson(strAttriDataSrc.c_str(),strDataSrc.c_str());
				jcp->WriteJson(strAttriTaskLst.c_str(),string(elm[strAttriTaskLst]).c_str());
				jcp->WriteJson(strAttriStartTime.c_str(),string(elm[strAttriStartTime]).c_str());
				jcp->WriteJson(strAttriEndTime.c_str(),string(elm[strAttriEndTime]).c_str());
				jcp->WriteJson(strAttriIdx.c_str(),string(elm[strAttriIdx]).c_str());
				jcp->EndObject();
			}
		}
	}
	jcp->EndArray();

	//数据库表初始化执行
	DbExeInitFun(move(strCreatSql),move(strSelectSql),move(strInsertSql),move(jcp));
}

void SqliteDbSvr::TaskCfgTableDbOpt(bool bInit)
{
	vector<Json> vecCfg;
	bInit ? SCCfgMng.GetExFactorySysCfg(strTaskNode,vecCfg) : SCCfgMng.GetSysCfgJsonObj(strTaskNode,vecCfg);
	if(vecCfg.empty())
	{
		return;
	}

	string&& strTableName = string(strTaskNode);
	string&& strCreatSql = string_format("create table if not exists %s(id INTEGER PRIMARY KEY autoincrement,node TEXT,type TEXT,lst TEXT,idx TEXT);",strTableName.c_str());
	string&& strSelectSql = string_format("select * from %s;",strTableName.c_str());
	string&& strInsertSql = string_format("INSERT INTO %s(id,node,type,lst,idx) VALUES(?,?,?,?,?);",strTableName.c_str());
	
	IF_SQLITE::CJsonCppShrPtr jcp(new CJsonCpp());
	jcp->StartArray();
	for (auto val : vecCfg)
	{
		for(auto it = val.begin();it != val.end();it++)
		{
			string&& strSubNode = string(it.key());
			for(auto elm : it.value())
			{
				jcp->StartObject();
				jcp->WriteJson(strAttriId.c_str(),nullptr);
				jcp->WriteJson(strAttriNode.c_str(), strSubNode.c_str());
				jcp->WriteJson(strAttriType.c_str(),string(elm[strAttriType]).c_str());
				jcp->WriteJson(strAttriLst.c_str(),string(elm[strAttriLst]).c_str());
				jcp->WriteJson(strAttriIdx.c_str(),string(elm[strAttriIdx]).c_str());
				jcp->EndObject();
			}
		}
	}
	jcp->EndArray();

	//数据库表初始化执行
	DbExeInitFun(move(strCreatSql),move(strSelectSql),move(strInsertSql),move(jcp));
}

void SqliteDbSvr::AlgCfgTableDbOpt(bool bInit)
{
	vector<Json> vecCfg;
	bInit ? SCCfgMng.GetExFactorySysCfg(strAlgNode,vecCfg) : SCCfgMng.GetSysCfgJsonObj(strAlgNode,vecCfg);
	if(vecCfg.empty())
	{
		return;
	}

	string&& strTableName = string(strAlgNode);
	string&& strCreatSql = string_format("create table if not exists %s(id INTEGER PRIMARY KEY autoincrement,node TEXT,type TEXT,name TEXT,version TEXT,model TEXT,params TEXT,enable TEXT,idx TEXT,desc TEXT);",strTableName.c_str());
	string&& strSelectSql = string_format("select * from %s;",strTableName.c_str());
	string&& strInsertSql = string_format("INSERT INTO %s(id,node,type,name,version,model,params,enable,idx,desc) VALUES(?,?,?,?,?,?,?,?,?,?);",strTableName.c_str());

    IF_SQLITE::CJsonCppShrPtr jcp(new CJsonCpp());
	jcp->StartArray();
	for (auto val : vecCfg)
	{
		for(auto it = val.begin();it != val.end();it++)
		{
			string&& strSubNode = string(it.key());
			for(auto elm : it.value())
			{
				jcp->StartObject();
				jcp->WriteJson(strAttriId.c_str(),nullptr);
				jcp->WriteJson(strAttriNode.c_str(), strSubNode.c_str());
				jcp->WriteJson(strAttriType.c_str(),string(elm[strAttriType]).c_str());
				jcp->WriteJson(strAttriName.c_str(),string(elm[strAttriName]).c_str());
				jcp->WriteJson(strAttriVersion.c_str(),string(elm[strAttriVersion]).c_str());
				
				string strModel("");
				ParseSubElmFun(strModel,string("model"),elm);
				strModel += string("&");
				jcp->WriteJson(strAttriModel.c_str(),strModel.c_str());

				string strParams("");
				ParseSubElmFun(strParams,strAttriParams.c_str(),elm);
				strParams += string("&");
				jcp->WriteJson(strAttriParams.c_str(),strParams.c_str());

				jcp->WriteJson(strAttriEnable.c_str(), string(elm[strAttriEnable]).c_str());
				jcp->WriteJson(strAttriIdx.c_str(),string(elm[strAttriIdx]).c_str());
				jcp->WriteJson(strAttriDesc.c_str(),string(elm[strAttriDesc]).c_str());
				jcp->EndObject();
			}
		}
	}
	jcp->EndArray();

	//数据库表初始化执行
	DbExeInitFun(move(strCreatSql),move(strSelectSql),move(strInsertSql),move(jcp));
}

void SqliteDbSvr::DbTransferCfgTableDOpt(tuple<string,bool>&& tpParam)
{
	string strNode;
	bool bInit;
	tie(strNode,bInit) = move(tpParam);
	
	vector<Json> vecCfg;
	(bInit) ? SCCfgMng.GetExFactorySysCfg(strNode,vecCfg) : SCCfgMng.GetSysCfgJsonObj(strNode,vecCfg);
	if(vecCfg.empty())
	{
		return;
	}

	string&& strTableName = string(strNode);
	string&& strCreatSql = string_format("create table if not exists %s(id INTEGER PRIMARY KEY autoincrement,node TEXT,type TEXT,ip TEXT,port TEXT,params TEXT,enable TEXT,idx TEXT,desc TEXT);",strTableName.c_str());
	string&& strSelectSql = string_format("select * from %s;",strTableName.c_str());
	string&& strInsertSql = string_format("INSERT INTO %s(id,node,type,ip,port,params,enable,idx,desc) VALUES(?,?,?,?,?,?,?,?,?);",strTableName.c_str());

	IF_SQLITE::CJsonCppShrPtr jcp(new CJsonCpp());
	jcp->StartArray();
	for (auto val : vecCfg)
	{
		for(auto it = val.begin();it != val.end();it++)
		{
			string&& strSubNode = string(it.key());
			for(auto elm : it.value())
			{
				jcp->StartObject();
				jcp->WriteJson(strAttriId.c_str(),nullptr);
				jcp->WriteJson(strAttriNode.c_str(), strSubNode.c_str());
				jcp->WriteJson(strAttriType.c_str(), string(elm[strAttriType]).c_str());
				jcp->WriteJson(strAttriIp.c_str(), string(elm[strAttriIp]).c_str());
				jcp->WriteJson(strAttriPort.c_str(), string(elm[strAttriPort]).c_str());

				string strParams("");
				ParseSubElmFun(strParams,strAttriParams.c_str(),elm);
				strParams += string("&");
				jcp->WriteJson(strAttriParams.c_str(),strParams.c_str());

				jcp->WriteJson(strAttriEnable.c_str(), string(elm[strAttriEnable]).c_str());
				jcp->WriteJson(strAttriIdx.c_str(),string(elm[strAttriIdx]).c_str());
				jcp->WriteJson(strAttriDesc.c_str(),string(elm[strAttriDesc]).c_str());
				jcp->EndObject();
			}
		}
	}
	jcp->EndArray();

	//数据库表初始化执行
	DbExeInitFun(move(strCreatSql),move(strSelectSql),move(strInsertSql),move(jcp));
}

void SqliteDbSvr::SysCfgDBOpt(bool bInit)
{
	//应用配置表初始化
	AppCfgTableDbOpt(bInit);

	//任务列表配置表初始化
	TaskCfgTableDbOpt(bInit);

	//算法库配置表初始化
	AlgCfgTableDbOpt(bInit);

	//数据库和传输配置表初始化
	auto&& tpTransfer = make_tuple<string,bool>(string(strTransferNode),bool(bInit));
	auto&& tpDb = make_tuple<string,bool>(string(strDbNode),bool(bInit));
	FuncInitLst(bind(&SqliteDbSvr::DbTransferCfgTableDOpt,this,placeholders::_1),move(tpTransfer),move(tpDb));

	//记录配置表初始化
	if(bInit)
	{
		ReportTableOpt();
	}
}

void SqliteDbSvr::ReportTableOpt()
{
	string&& strTableName = "Report";
	string&& strCreatSql = string_format("create table if not exists %s(id INTEGER PRIMARY KEY autoincrement,idx TEXT,task_flag TEXT,alg_idx TEXT,alert_type TEXT,alert_date TEXT,data_src TEXT,img TEXT);",strTableName.c_str());
	string&& strSelectSql = string_format("select * from %s;",strTableName.c_str());
	IF_SQLITE::CJsonCppShrPtr jcp(nullptr);

	//数据库表初始化执行
	DbExeInitFun(move(strCreatSql),move(strSelectSql),move(""),move(jcp));
}

bool SqliteDbSvr::SysCfgDBInit()
{
	//获取配置对象
	vector<nlohmann::json> vecSqlitefg;
	string&& strKey = strDbNode + strDbSqliteNode;
	SCCfgMng.GetExFactorySysCfg(strKey,vecSqlitefg);

	for (auto val : vecSqlitefg)
	{
		//数据库操作消息订阅
		string&& strDBIdx = string(val[strAttriType]) + string(val[strAttriIdx]);
		string&& strSaveTopic = SaveDBTopic(strDBIdx);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string& strSql,const CJsonCpp& jData)->bool {return SqliteExcecuteJson(strSql,jData);}, strSaveTopic);

		string&& strGetTopic = ReadDBTopic(strDBIdx);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strSql, rapidjson::Document& jData) ->void {SqliteQuery(strSql, jData);}, strGetTopic);

		string&& strDelTopic = DelDBTopic(strDBIdx);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strSql) ->bool { SqliteExcecute(strSql);}, strDelTopic);
	}
	
	//系统配置表初始化
	SysCfgDBOpt();

	return true;
}

bool SqliteDbSvr::SqliteExcecuteJson(const string& strSql,const CJsonCpp& jData)
{
	return IF_SQLITE::SqliteExcecuteJson(strSql,jData);
}

void SqliteDbSvr::SqliteQuery(const string& strSql,rapidjson::Document& jData)
{
	return IF_SQLITE::SqliteQuery(strSql,jData);
}

bool SqliteDbSvr::SqliteExcecute(const string& strSql)
{
	return IF_SQLITE::SqliteExcecute(strSql);
}