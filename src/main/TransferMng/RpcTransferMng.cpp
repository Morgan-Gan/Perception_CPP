#include "RpcTransferMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "log4cxx/Loging.h"
#include "SubTopic.h"
#include "comm/ReleaseThread.h"

using namespace std;
using namespace MAIN_MNG;
using namespace common_template;
using namespace common_cmmobj;

//启动rpc服务
bool CRpcTransferMng::StartRpcli()
{
    	//获取配置对象
	vector<nlohmann::json> vecReportTransCfg;
	string&& strKey = strTransferNode + strTransferNorthRpcNode + strAttriRpcCliValue;
	SCCfgMng.GetSysCfgJsonObj(strKey,vecReportTransCfg);

	//字段解析
	for (auto elm : vecReportTransCfg)
	{
		//配置字段
		string&& strIp = elm[strAttriIp];
		string&& strPort = elm[strAttriPort];
		string&& strMaxConn = elm[strAttriParams][strAttriConn];

		//Rpc模块初始化
		IF_RPC_SERVICE::RpcInit((char*)strIp.c_str(), ::atoi(strPort.c_str()), ::atoi(strMaxConn.c_str()));

		//Rpc远程调用订阅
		m_strRpcServiceTopic = RpcServiceTopic(std::move(strIp), std::move(strPort));
		m_strRpcSendDataTopic = RpcSendTopic(std::move(strIp), std::move(strPort));
		
		SCMsgBusMng.GetMsgBus()->Attach([this](Any && AnyRpcClient, Any && AnyRpcService) {return GetRpcService(std::move(AnyRpcClient), std::move(AnyRpcService)); }, m_strRpcServiceTopic);
		SCMsgBusMng.GetMsgBus()->Attach([this](IF_RPC_SERVICE::Func&& func, const std::string& strFuncName, Any&& AnyRpcClient) ->bool {return SendRpcData(std::move(func), std::move(AnyRpcClient), strFuncName); }, m_strRpcSendDataTopic);
	}

	return true;

	//上线注册
	if (!Online())
	{
		return false;
	}

	//建立心跳线程
	m_ptrOnlineThread = ThreadShrPtr(new common_cmmobj::CThread(60, 2, "AliveLoop",-1,std::bind(&CRpcTransferMng::AliveLoop, this)));

	return true;
}

//发送数据到rpc服务器
bool CRpcTransferMng::SendRpcData(IF_RPC_SERVICE::Func&& func, Any&& AnyRpcClient, const std::string& strFuncName)
{
	return IF_RPC_SERVICE::CallRpcFunc(std::move(func), strFuncName, std::move(AnyRpcClient));
}

bool CRpcTransferMng::GetRpcService(Any && AnyRpcClient, Any && AnyRpcService)
{
	return IF_RPC_SERVICE::GetRpc(std::move(AnyRpcClient), std::move(AnyRpcService));
}

//设备上线注册
bool CRpcTransferMng::Online()
{
	//随机id
	srand((unsigned)time(NULL));
	char cId[9] = { 0 };
	sprintf((char*)cId, "%d", rand());
	m_strAccessId = string(cId);

	//获取机器的信息
	getlocalinfo(m_strIpAddress, m_strMacAddress);
	LOG_INFO("systerm") << string_format("rand:%s; macAddr:%s; ipAddr:%s\n", m_strAccessId.c_str(), m_strMacAddress.c_str(), m_strIpAddress.c_str()).c_str();

	//远程调用
	return RpcWrap("online",m_strRpcServiceTopic,m_strRpcSendDataTopic, SCMsgBusMng.GetMsgBus(),&AccessServiceClient::online, m_strAccessId, m_strMacAddress, m_strIpAddress);
}

//心跳线程
void CRpcTransferMng::AliveLoop()
{
	//远程调用
	RpcWrap("alive", m_strRpcServiceTopic,m_strRpcSendDataTopic, SCMsgBusMng.GetMsgBus(),&AccessServiceClient::alive, m_strAccessId);
}

CRpcTransferMng::CRpcTransferMng()
{
    ReleaseThread(m_ptrOnlineThread);
}