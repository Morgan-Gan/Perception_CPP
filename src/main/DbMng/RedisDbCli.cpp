#include "RedisDbCli.h"
#include "IRedisServiceModule.h"
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

bool CRedisDbCli::RedisInit()
{
	//获取配置对象
	vector<nlohmann::json> vecSaveRedisCfg;
	string&& strKey = strDbNode + strDbRedisNode;
	SCCfgMng.GetSysCfgJsonObj(strKey,vecSaveRedisCfg);

	//字段解析
	for (auto elm : vecSaveRedisCfg)
	{
		if("0" == string(elm[strAttriEnable]))
		{
			continue;
		}
		
		string&& strIp = elm[strAttriIp];
		string&& strPort = elm[strAttriPort];

		string&& strPwd = elm[strAttriParams][strAttriPwd];
		string&& strConnTimeout = elm[strAttriParams][strAttriConnTimeOut];
		string&& strRWTimeout = elm[strAttriParams][strAttriRwTimeOut];
		string&& strInitConn = elm[strAttriParams][strAttriInitConn];
		string&& strMaxConn = elm[strAttriParams][strAttriMaxConn];

		//redis初始化
		bool bRet = IF_REDIS::RedisInit(strIp, strPwd, atoi(strPort.c_str()), atoi(strConnTimeout.c_str()), atoi(strRWTimeout.c_str()), atoi(strMaxConn.c_str()), atoi(strInitConn.c_str()));
		if (!bRet)
		{
			return false;
		}

		//消息订阅
		string&& strDBIdx = string(elm[strAttriType]) + string(elm[strAttriIdx]);
		string&& strSaveTopic = SaveDBTopic(strDBIdx);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strKey, const string& strData)->bool {return RedisSaveData(strKey, strData); }, strSaveTopic);

		string&& strGetTopic = ReadDBTopic(strDBIdx);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strKey, vector<string>& vecData) {RedisReadData(strKey, vecData); }, strGetTopic);

		string&& strDelTopic = DelDBTopic(strDBIdx);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strKey) {RedisDelData(strKey); }, strDelTopic);

		//目前仅支持一个redis服务器
		break;
	}

	return true;
}

bool CRedisDbCli::RedisSaveData(const string & strKey, const string & strData)
{
	bool bRet = IF_REDIS::RedisSaveData(strKey, strData);

	LOG_DEBUG("systerm") << string_format("key(%s) save data to redis %s\n", strKey.c_str(), bRet ? "successful" : "fail");
	return bRet;
}

void CRedisDbCli::RedisReadData(const string & strKey, vector<string>& vecData)
{
	IF_REDIS::RedisReadData(strKey, vecData);
}

void CRedisDbCli::RedisDelData(const std::string & strKey)
{
	IF_REDIS::RedisDelData(strKey);
}