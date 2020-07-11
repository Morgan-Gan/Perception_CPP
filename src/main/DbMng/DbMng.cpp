#include "DbMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "CfgMng/CfgMng.h"
#include "DeviceMng/DeviceMng.h"
#include "IRedisServiceModule.h"
#include "log4cxx/Loging.h"
#include "SubTopic.h"
#include "boost/BoostFun.h"

using namespace std;
using namespace MAIN_MNG;
using namespace common_cmmobj;
using namespace common_template;

bool CDbMng::InitDb()
{
	return InitRedis();
}

bool CDbMng::InitRedis()
{
	//获取配置对象
	vector<nlohmann::json> vecSaveRedisCfg;
	string&& strKey = strDbNode + strDbRemoteNode + strRedisCliType;
	SCCfgMng.GetCfgJsonObj(strKey,vecSaveRedisCfg);

	//字段解析
	for (auto elm : vecSaveRedisCfg)
	{
		string&& strIp = elm["ip"];
		string&& strPort = elm["port"];
		string&& strPwd = elm["pwd"];

		string&& strConnTimeout = elm["conn_timeout"];
		string&& strRWTimeout = elm["rw_timeout"];
		string&& strInitConn = elm["init_conn"];
		string&& strMaxConn = elm["max_conn"];

		//redis初始化
		bool bRet = IF_REDIS::InitRedis(strIp, strPwd, atoi(strPort.c_str()), atoi(strConnTimeout.c_str()), atoi(strRWTimeout.c_str()), atoi(strMaxConn.c_str()), atoi(strInitConn.c_str()));
		if (!bRet)
		{
			return false;
		}

		//消息订阅
		string&& strSaveTopic = GetRedisSaveTopic(strIp, strPort);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strKey, const string& strData)->bool {return SaveDataToRedis(strKey, strData); }, strSaveTopic);

		string&& strGetTopic = GetRedisReadTopic(strIp, strPort);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strKey, vector<string>& vecData) {GetDataFromRedis(strKey, vecData); }, strGetTopic);

		string&& strDelTopic = GetRedisDelTopic(strIp, strPort);
		SCMsgBusMng.GetMsgBus()->Attach([this](const string & strKey) {DelDataFromRedis(strKey); }, strDelTopic);

		//目前仅支持一个redis服务器
		break;
	}

	return true;
}

bool CDbMng::SaveDataToRedis(const string & strKey, const string & strData)
{
	bool bRet = IF_REDIS::SaveDataToRedis(strKey, strData);

	LOG_DEBUG("systerm") << string_format("key(%s) save data to redis %s\n", strKey.c_str(), bRet ? "successful" : "fail");
	return bRet;
}

void CDbMng::GetDataFromRedis(const string & strKey, vector<string>& vecData)
{
	IF_REDIS::GetDataFromRedis(strKey, vecData);
}

void CDbMng::DelDataFromRedis(const std::string & strKey)
{
	IF_REDIS::DelDataFromRedis(strKey);
}

CDbMng::CDbMng()
{
}

CDbMng::~CDbMng()
{
}