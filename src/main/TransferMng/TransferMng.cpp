#include "TransferMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "DeviceMng/DeviceMng.h"
#include "log4cxx/Loging.h"
#include "SubTopic.h"
#include "VsTransferMng.h"

using namespace std;
using namespace MAIN_MNG;
using namespace common_template;
using namespace common_cmmobj;

/*南向通信启动*/
bool CTransferMng::StartSouthTrans()
{
	//启动串口通信
	StartSerial();

	//启动视频拉流通信
	StartRtspCli();

	return StartTcpServer();
}

//串口通信
void CTransferMng::StartSerial()
{
}

//rstp视频拉流客户端通信
void CTransferMng::StartRtspCli()
{
	SCVsTransferMng.StartCameras();
}

//tcp服务器通信
bool CTransferMng::StartTcpServer()
{
	//设置tcp通信回调
	if (!m_pTcpServiceModule)
	{
		m_pTcpServiceModule = GetTCPServiceModuleInstance();
		m_pTcpServiceModule->SetCallbackObj(this);
	}

	//获取tcp通信方式配置对象
	vector<tuple<unsigned short, int>> vecServerCfg;
	vector<nlohmann::json> vecSouthTcpTransCfg;
	string&& strKey = strTransferNode + strTransferSouthNode + strTcpSrvType;
	SCCfgMng.GetCfgJsonObj(strKey,vecSouthTcpTransCfg);
	for (nlohmann::json elm : vecSouthTcpTransCfg)
	{
		TcpServerAttach(std::move(elm), vecServerCfg);
	}

	//启动tcp服务器
	return m_pTcpServiceModule->StartTCPServer(std::move(vecServerCfg));
}

////订阅关闭客户端和发送客户主题
void CTransferMng::TcpServerAttach(nlohmann::json&& elm, vector<tuple<unsigned short, int>>& vecServerCfg)
{
	string&& strType = elm["type"];
	if (0 != strTcpSrvType.compare(strType))
	{
		return;
	}
	string&& strPort = elm["port"];
	string&& strThreads = elm["threads"];
	tuple<unsigned short, int> Tp = make_tuple(atoi(strPort.c_str()), atoi(strThreads.c_str()));
	vecServerCfg.emplace_back(Tp);

	string strTcpCliCloseTopic = GetTcpCliCloseTopic(strPort);
	string strTcpCliSendMsgTopic = GetTcpCliSendMsgTopic(strPort);

	SCMsgBusMng.GetMsgBus()->Attach([this](const string & strConnKey, const int & s32TcpServerPort)->void {return CloseTcpConnect(strConnKey, s32TcpServerPort); }, strTcpCliCloseTopic);
	SCMsgBusMng.GetMsgBus()->Attach([this](const string & strConnKey, const string& strData, const int & s32TcpServerPort)->bool {return SendTcpData(strConnKey, strData, s32TcpServerPort); }, strTcpCliSendMsgTopic);
}

////tcp服务器收到连接回调
void CTransferMng::OnTcpConnect(const string & strPeerConnKey, const string & strLocalConnKey, bool bStatus)
{
	string&& strTopic = GetTcpCliConnTopic(strLocalConnKey.substr(strLocalConnKey.find(':') + 1));
	SCMsgBusMng.GetMsgBus()->SendReq<void, const string&, bool&>(strPeerConnKey, bStatus, strTopic);
}

////tcp服务器收到信息回调
void CTransferMng::OnTcpMessage(const string & strPeerConnKey, const string & strLocalConnKey, const char * pData, const int nDataLen)
{
	string&& strTopic = GetTcpSrvRecMsgTopic(strLocalConnKey.substr(strLocalConnKey.find(':') + 1));
	SCMsgBusMng.GetMsgBus()->SendReq<void, const string&, const char*&, const int&>(strPeerConnKey, pData, nDataLen, strTopic);
}

////向tcp客户端发送数据
bool CTransferMng::SendTcpData(const string & strConnKey, const string& strData, const int & s32TcpServerPort)
{
	return m_pTcpServiceModule->SendData(strConnKey, strData, s32TcpServerPort);
}

////关闭tcp客户端连接
void CTransferMng::CloseTcpConnect(const string & strConnKey, const int & s32TcpServerPort)
{
	return m_pTcpServiceModule->CloseConnect(strConnKey, s32TcpServerPort);
}

/*北向通信启动*/
bool CTransferMng::StartNorthTrans()
{
	//启动http客户端通信
	StartHttpCli();

	//启动rpc客户端通信
	return StartRpcli();
}

//http cli
void CTransferMng::StartHttpCli()
{
}

//rpc通信
bool CTransferMng::StartRpcli()
{
	//获取配置对象
	vector<nlohmann::json> vecReportTransCfg;
	string&& strKey = strTransferNode + strTransferNorthNode + strRpcType;
	SCCfgMng.GetCfgJsonObj(strKey,vecReportTransCfg);

	//字段解析
	for (auto elm : vecReportTransCfg)
	{
		//配置字段
		string&& strIp = elm["ip"];
		string&& strPort = elm["port"];
		string&& strMaxConn = elm["conn"];

		//Rpc模块初始化
		IF_RPC_SERVICE::RpcInit((char*)strIp.c_str(), ::atoi(strPort.c_str()), ::atoi(strMaxConn.c_str()));

		//Rpc远程调用订阅
		m_strRpcServiceTopic = GetRpcServiceTopic(std::move(strIp), std::move(strPort));
		m_strRpcSendDataTopic = GetRpcSendTopic(std::move(strIp), std::move(strPort));
		
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
	m_ptrOnlineThread = ThreadShrPtr(new common_cmmobj::CThread(std::bind(&CTransferMng::AliveLoop, this), 60, 2, "AliveLoop"));

	return true;
}

////发送数据到rpc服务器
bool CTransferMng::SendRpcData(IF_RPC_SERVICE::Func&& func, Any&& AnyRpcClient, const std::string& strFuncName)
{
	return IF_RPC_SERVICE::CallRpcFunc(std::move(func), strFuncName, std::move(AnyRpcClient));
}

bool CTransferMng::GetRpcService(Any && AnyRpcClient, Any && AnyRpcService)
{
	return IF_RPC_SERVICE::GetRpc(std::move(AnyRpcClient), std::move(AnyRpcService));
}

////设备上线注册
bool CTransferMng::Online()
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

////心跳线程
void CTransferMng::AliveLoop()
{
	//远程调用
	RpcWrap("alive", m_strRpcServiceTopic,m_strRpcSendDataTopic, SCMsgBusMng.GetMsgBus(),&AccessServiceClient::alive, m_strAccessId);
}

CTransferMng::CTransferMng() : m_pTcpServiceModule(nullptr)
{
}

CTransferMng::~CTransferMng()
{
}