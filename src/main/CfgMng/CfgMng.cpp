#include "CfgMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "SubTopic.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "comm/FuncInitLst.h"
#include "DbMng/DbMng.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace MAIN_MNG;
using namespace common_cmmobj;
using namespace common_template;

const string strCfgFilePath = string("./config/config.json");

bool CCfgMng::LoadCfg()
{
	//加载出厂配置文件
	LoadSysCfgFromFile(strCfgFilePath);
	
	//加载数据库配置文件
	LoadSysCfgFromDb();

	//订阅配置工具消息
	vector<Json> vecTcpServerCfg;
	GetSysCfgJsonObj(strTransferNode + strTransferSouthTcpSvrNode,vecTcpServerCfg);
	for(auto var : vecTcpServerCfg)
	{
		auto func = [this](const std::string& strTcpSvrPort, const std::string& strCliConn,const unsigned int& u32Cmd,const char*& pData, const int& s32DataLen)->void {return RecTcpProtocolMsg(strTcpSvrPort, strCliConn,u32Cmd,pData,s32DataLen); };
		SCMsgBusMng.GetMsgBus()->Attach(move(func), TcpSrvRecMsgTopic(var[strAttriPort]));
	}
	return true;
}

void CCfgMng::GetSysCfgJsonObj(const std::string &strKey, std::vector<Json> &lstCfg)
{
	if (m_mapSysCfgInDbShrPtr->end() != m_mapSysCfgInDbShrPtr->find(strKey))
	{
		(*m_mapSysCfgInDbShrPtr.get())[strKey].GetJsonObj(lstCfg);
	}
}

bool CCfgMng::GetSysCfgJsonObj(const std::string &strKey, Json& cfg)
{
	if(m_mapSysCfgInDbShrPtr->end() != m_mapSysCfgInDbShrPtr->find(strKey))
	{
		return (*m_mapSysCfgInDbShrPtr.get())[strKey].GetJsonObj(cfg);
	}

	return false;
}

void CCfgMng::GetExFactorySysCfg(const std::string &strKey, std::vector<Json> &lstCfg)
{
	if (m_mapSysCfgInFileShrPtr->end() != m_mapSysCfgInFileShrPtr->find(strKey))
	{
		(*m_mapSysCfgInFileShrPtr.get())[strKey].GetJsonObj(lstCfg);
	}
}

bool CCfgMng::GetExFactorySysCfg(const std::string &strKey, Json& cfg)
{
	if(m_mapSysCfgInFileShrPtr->end() != m_mapSysCfgInFileShrPtr->find(strKey))
	{
		return (*m_mapSysCfgInFileShrPtr.get())[strKey].GetJsonObj(cfg);
	}

	return false;
}

//加载出厂json基本配置
void CCfgMng::LoadSysCfgFromFile(const string &strPath)
{
	//从文件读取json格式配置
	string&& strJson = ReadFileData(strPath);
	if (strJson.empty())
	{
		return;
	}
	m_jSysCfgInFile = Json::parse(strJson.c_str());

	//解析出厂配置
	ParseJson(m_jSysCfgInFile,m_mapSysCfgInFileShrPtr);

	//恢复出厂配置到数据库
	SCDbMng.SysCfgDBInit();
}

auto SpliteStrFun = [](vector<Json>& vecJson,const string& strValue)
{
	vector<string> lstGrpStr;
	Strtok(strValue, "&", lstGrpStr);
	for (auto group : lstGrpStr)
	{
		Json jTmp;
		vector<string> lstStr;
		Strtok(group, "|", lstStr);
		for(auto val : lstStr)
		{
			vector<string> lstSubStr;
			Strtok(val, ":", lstSubStr);
			if(0 < lstSubStr.size())
			{
				string&& strAttributeVal = (1 == lstSubStr.size()) ? "" : lstSubStr[1];
				jTmp[lstSubStr[0]] = strAttributeVal;
			}
		}
		vecJson.push_back(jTmp);
	}
};

void CCfgMng::LoadSysCfgTableFromDb(const string& strTableName)
{
	Json jSqliteSvr;
	if(!GetExFactorySysCfg(strDbNode + strDbSqliteNode,jSqliteSvr))
	{
		return;
	}

	string&& strTransferSql = string_format("select * from %s;",strTableName.c_str());
	string&& strReadSqliteTopic = ReadDBTopic(string(jSqliteSvr[strAttriType]) + string(jSqliteSvr[strAttriIdx]));
	rapidjson::Document doc;
	SCMsgBusMng.GetMsgBus()->SendReq<void,const string&,rapidjson::Document&>(move(strTransferSql),doc,move(strReadSqliteTopic));

	size_t len = doc.Size();
	if(0 >= len)
	{
		return;
	}

	using MemberIterType = rapidjson::Value::ConstMemberIterator;
	for (size_t i = 0; i < len; i++)
	{
		Json jSubNode;
		string strSubNodeName("");
		for (MemberIterType ite = doc[i].MemberBegin();ite != doc[i].MemberEnd();)
		{
			//获取json值
			const char* key = doc[i].GetKey(ite++);
			if(!key)
			{
				break;
			}

			if(strAttriId == string(key))
			{
				continue;
			}

			auto& t = doc[i][key];
			string strValue(t.GetString());

			if(strAttriNode == string(key))
			{
				strSubNodeName = strValue;
				continue;
			}

			//分割字符串重组json节点数据
			vector<Json> vecParam;
			if(strAttriDataSrc == string(key))
			{
				SpliteStrFun(vecParam,strValue);
				jSubNode[key] = vecParam;
				continue;
			}

			if(strAttriModel == string(key) || strAttriParams == string(key))
			{
				SpliteStrFun(vecParam,strValue);
				jSubNode[key] = (vecParam.empty()) ? Json({}) : vecParam[0];
				continue;
			}

			//赋属性值
			jSubNode[key] = strValue;
		}

		m_jSysCfgInDb[strTableName][strSubNodeName].push_back(jSubNode);
	}
}

//加载数据库中系统的基本配置
void CCfgMng::LoadSysCfgFromDb()
{
	//加载数据库配置为json格式
	FuncInitLst(bind(&CCfgMng::LoadSysCfgTableFromDb,this,placeholders::_1),strTransferNode,strDbNode,strAlgNode,strTaskNode,strAppNode);

	//解析json配置并保存内存中
	ParseJson(m_jSysCfgInDb,m_mapSysCfgInDbShrPtr);
}

void CCfgMng::ParseJson(const Json& jObj,const CfgObjMapTypeShrPtr& mapExFactoryCfgObjShrPtr)
{
	TupleType&& transTuple = make_tuple<string,Json,CfgObjMapTypeShrPtr>(string(strTransferNode),Json(jObj),CfgObjMapTypeShrPtr(mapExFactoryCfgObjShrPtr));
	TupleType&& algTuple = make_tuple<string,Json,CfgObjMapTypeShrPtr>(string(strAlgNode),Json(jObj),CfgObjMapTypeShrPtr(mapExFactoryCfgObjShrPtr));
	TupleType&& dbTuple = make_tuple<string,Json,CfgObjMapTypeShrPtr>(string(strDbNode),Json(jObj),CfgObjMapTypeShrPtr(mapExFactoryCfgObjShrPtr));
	TupleType&& appTuple = make_tuple<string,Json,CfgObjMapTypeShrPtr>(string(strAppNode),Json(jObj),CfgObjMapTypeShrPtr(mapExFactoryCfgObjShrPtr));
	TupleType&& taskTuple = make_tuple<string,Json,CfgObjMapTypeShrPtr>(string(strTaskNode),Json(jObj),CfgObjMapTypeShrPtr(mapExFactoryCfgObjShrPtr));
	FuncInitLst(bind(&CCfgMng::ParseJsonNodeObj,this,placeholders::_1),move(transTuple),move(algTuple),move(dbTuple),move(appTuple),move(taskTuple));
}

void CCfgMng::ParseJsonNodeObj(TupleType&& jCfg)
{
	string strNode;
	Json jObj;
	CfgObjMapTypeShrPtr cfgMapShrPtr;
	tie(strNode,jObj,cfgMapShrPtr) = move(jCfg);

	//一级节点保存
	SaveJsonObj(strNode,jObj[strNode],cfgMapShrPtr);

	//二级节点
	for (auto it = jObj[strNode].begin(); it != jObj[strNode].end(); ++it)
	{
		string&& strSubNode = string(it.key());
		for (auto elm : it.value())
		{
			//节点方式
			string&& strNodeKey = strNode + strSubNode;
			SaveJsonObj(strNodeKey,elm,cfgMapShrPtr);

			//节点+类型方式
			string&& strTypeKey = strNode + strSubNode + string(elm[strAttriType]);
			SaveJsonObj(strTypeKey,elm,cfgMapShrPtr);

			//唯一标识方式
			string&& strIdxKey = string(elm[strAttriType]) + string(elm[strAttriIdx]);
			SaveJsonObj(strIdxKey,elm,cfgMapShrPtr);
		}
	}
}

void CCfgMng::SaveJsonObj(const string& strKey,Json& elm,const CfgObjMapTypeShrPtr& cfgMapShrPtr)
{
	if(cfgMapShrPtr->end() == cfgMapShrPtr->find(strKey))
	{
		CfgObj cfgObj;
		cfgObj.SaveJsonObj(elm);
		cfgMapShrPtr->insert(pair<string, CfgObj>(strKey, cfgObj));
	}
	else
	{
		(*cfgMapShrPtr.get())[strKey].SaveJsonObj(elm);
	}
}

void CCfgMng::CleanSysCfgMap()
{
	(*(m_mapSysCfgInDbShrPtr.get())).clear();
}

void CCfgMng::SysCfgUpdate(const string& strRootNode,Json& jSubNode)
{
	//当idx=-1时为新创建配置,因此需要重新分配
	for(auto ite = jSubNode.begin();ite != jSubNode.end();ite++)
	{
		string&& strSubNodeKey = string(ite.key());
		int s32MaxIdx = 0;
		for(auto& obj : m_jSysCfgInDb[strRootNode][strSubNodeKey])
		{
			string&& strIdx = obj[strAttriIdx];
			if("-1" != strIdx)
			{
				int&& s32Idx = atoi(strIdx.c_str());
				s32MaxIdx = (s32Idx > s32MaxIdx) ? s32Idx : s32MaxIdx;
			}
		}

		for(auto& obj : jSubNode[strSubNodeKey])
		{
			string&& strIdx = obj[strAttriIdx];
			if("-1" == strIdx)
			{
				obj[strAttriIdx] = to_string(++s32MaxIdx);
			}
		}
	}

	//内存中系统配置更新
	m_jSysCfgInDb.erase(m_jSysCfgInDb.find(strRootNode));
	m_jSysCfgInDb[strRootNode] = jSubNode;

	CleanSysCfgMap();
	ParseJson(m_jSysCfgInDb,m_mapSysCfgInDbShrPtr);

	//数据库中系统配置更新
	Json jSqliteSvr;
	if(!GetExFactorySysCfg(strDbNode + strDbSqliteNode,jSqliteSvr))
	{
		return;
	}
	string&& strSql = string_format("delete from %s;",strRootNode);
	string&& strDelTopic = DelDBTopic(string(jSqliteSvr[strAttriType]) + string(jSqliteSvr[strAttriIdx]));
	SCMsgBusMng.GetMsgBus()->SendReq<bool,const string&>(strSql,strDelTopic);
	SCDbMng.SysCfgDBOpt(false);
}

void CCfgMng::RecTcpProtocolMsg(const string& strTcpSvrPort, const string& strCliConn,const unsigned int& u32Cmd,const char*& pData, const int& s32DataLen)
{
	if(!pData)
	{
		return;
	}

	string strJson(pData,s32DataLen); 

	bool bRsp = false;
	Json jRspData;

	switch(u32Cmd)
	{
	case 0x11:
	{
		jRspData[strTransferNode] = m_jSysCfgInDb[strTransferNode];
		jRspData[strDbNode] = m_jSysCfgInDb[strDbNode];

		Json jDetectAlgCfg = m_jSysCfgInDb[strAlgNode][strAlgDetectNode][0];
		Json jActionAlgCfg = m_jSysCfgInDb[strAlgNode][strAlgActionNode][0];

		string strLst = string(m_jSysCfgInDb[strTaskNode][strTaskDetectNode][0][strAttriLst]);
		jDetectAlgCfg[strAttriTaskLst] = string(m_jSysCfgInDb[strTaskNode][strTaskDetectNode][0][strAttriLst]);
		jActionAlgCfg[strAttriTaskLst] = string(m_jSysCfgInDb[strTaskNode][strTaskActionNode][0][strAttriLst]);

		jRspData[strAlgNode][strAlgDetectNode] = jDetectAlgCfg;
		jRspData[strAlgNode][strAlgActionNode] = jActionAlgCfg;
		bRsp = true;
	}
	break;
	case 0x12:
	{
		for(auto ite = m_jSysCfgInDb[strAppNode].begin();ite != m_jSysCfgInDb[strAppNode].end();ite++)
		{
			for(auto obj : ite.value())
			{
				Json jTaskNode;
				jTaskNode[strAttriTaskFlag] = string(obj[strAttriType]);
				jTaskNode[strAttriAlgIdx] = string(obj[strAttriAlgIdx]);

				for(auto subObj : obj[strAttriDataSrc])
				{
					string&& strTransId = string(subObj[strAttriTransferId]);
					Json jDataSrc;
					Json jDesc;
					if(!GetSysCfgJsonObj(strTransId,jDesc))
					{
						jDataSrc = Json({});
					}
					else
					{
						jDataSrc[strAttriTransferId] = strTransId;
						jDataSrc[strAttriDesc] = string(jDesc[strAttriDesc]);
					}
					jTaskNode[strAttriDataSrc].push_back(jDataSrc);
				}

				jTaskNode[strAttriTaskLst] = string(obj[strAttriTaskLst]);
				jTaskNode[strAttriStartTime] = string(obj[strAttriStartTime]);
				jTaskNode[strAttriEndTime] = string(obj[strAttriEndTime]);
				jTaskNode[strAttriIdx] = string(obj[strAttriIdx]);
				jRspData.push_back(jTaskNode);
			}
		}
		bRsp = true;
	}
	break;
	case 0x13:
	{
		//任务配置更新
		Json jSubNode;
		jSubNode[strAppCommSafeNode] = Json::parse(strJson);
		for(auto& obj : jSubNode[strAppCommSafeNode])
		{
			obj[strAttriType] = string(obj[strAttriTaskFlag]);
			obj.erase(obj.find(strAttriTaskFlag));
		}
		SysCfgUpdate(strAppNode,jSubNode);

		//回复协议
		jRspData[strAttriResult] = string(strAttriOk);
		bRsp = true;
	}
	break;
	case 0x14:
	{
		//传输配置更新
		Json jSubNode = Json::parse(strJson);
		SysCfgUpdate(strTransferNode,jSubNode);

		//回复协议
		jRspData[strAttriResult] = string(strAttriOk);
		bRsp = true;
	}
	break;
	case 0x15:
	{
		//数据库配置更新
		Json jSubNode = Json::parse(strJson);
		SysCfgUpdate(strDbNode,jSubNode);

		//回复协议
		jRspData[strAttriResult] = string(strAttriOk);
		bRsp = true;
	}
	break;
	default:
		break;
	}

	if(bRsp)
	{
		LOG_INFO("systerm") << string_format("req cmd(0x%02x) and json:%s\n",u32Cmd,strJson.c_str());
		string&& strRspData = jRspData.dump();
		LOG_INFO("systerm") << string_format("rsq cmd(0x%02x) and json:%s\n",u32Cmd,strRspData.c_str());
		SCMsgBusMng.GetMsgBus()->SendReq<bool,const int &,const string&, const int&,const string&,const int&>(atoi(strTcpSvrPort.c_str()),strCliConn,u32Cmd,move(strRspData),1,TcpSrvSendMsgTopic(strTcpSvrPort));
	}
}