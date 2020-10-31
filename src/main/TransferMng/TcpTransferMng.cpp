#include "TcpTransferMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "SubTopic.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace MAIN_MNG;
using namespace common_cmmobj;

bool CTcpTransferMng::StartTcpSvr() 
{ 
    //设置tcp通信回调
	if (!m_pTcpServiceModule)
	{
		m_pTcpServiceModule = GetTCPServiceModuleInstance();
		m_pTcpServiceModule->SetCallbackObj(this);
	}

	//获取tcp通信方式配置对象
	vector<nlohmann::json> vecSouthTcpTransCfg;
	string&& strKey = strTransferNode + strTransferSouthTcpSvrNode + strAttriTcpSrvValue;
	SCCfgMng.GetSysCfgJsonObj(strKey,vecSouthTcpTransCfg);
	if(vecSouthTcpTransCfg.empty())
	{
		return false;
	}

	//消息订阅
	vector<tuple<unsigned short, int>> vecServerCfg;
	for (nlohmann::json elm : vecSouthTcpTransCfg)
	{
		TcpServerAttach(std::move(elm), vecServerCfg);
	}

	//启动tcp服务器
	return m_pTcpServiceModule->StartTCPServer(std::move(vecServerCfg));
}

//订阅关闭客户端连接和发送消息给客户端主题
void CTcpTransferMng::TcpServerAttach(nlohmann::json&& elm, vector<tuple<unsigned short, int>>& vecServerCfg)
{
	string&& strType = elm[strAttriType];
	if (0 != strAttriTcpSrvValue.compare(strType))
	{
		return;
	}
	string&& strPort = elm[strAttriPort];
	string&& strThreads = elm[strAttriParams][strAttriThreads];
	tuple<unsigned short, int> Tp = make_tuple(atoi(strPort.c_str()), atoi(strThreads.c_str()));
	vecServerCfg.emplace_back(Tp);

	string&& strTcpSrvCloseConnTopic = TcpSrvCloseConnTopic(strPort);
	string&& strTcpSrvSendMsgTopic = TcpSrvSendMsgTopic(strPort);

	SCMsgBusMng.GetMsgBus()->Attach([this](const string & strConnKey, const int & s32TcpServerPort)->void {return CloseTcpConnect(strConnKey, s32TcpServerPort); }, strTcpSrvCloseConnTopic);
	SCMsgBusMng.GetMsgBus()->Attach([this](const int & s32TcpServerPort,const string & strConnKey, const int& s32Cmd,const string& strData,const int& s32SendType)->bool {return SendTcpData(s32TcpServerPort,strConnKey, s32Cmd,strData,s32SendType); }, strTcpSrvSendMsgTopic);
}

//tcp服务器收到连接回调
void CTcpTransferMng::OnTcpConnect(const string & strPeerConnKey, const string & strLocalConnKey, bool bStatus)
{
	//管理封解包器
	string&& strTcpServerPort = strLocalConnKey.substr(strLocalConnKey.find(':') + 1);
	string&& strKey = strTcpServerPort + strPeerConnKey;

	auto iter = m_mapNetTcpDataPkgPtr.find(strKey);
	if(m_mapNetTcpDataPkgPtr.end() == iter)
	{
		auto DistributeFunc = bind(&CTcpTransferMng::DistributProtocolData,this,placeholders::_1,placeholders::_2,placeholders::_3,placeholders::_4,placeholders::_5,placeholders::_6);
		NetTcpDatPkgShrPtr&& ptr = NetTcpDatPkgShrPtr(new CNetTcpDataPkg(move(strTcpServerPort),strPeerConnKey,DistributeFunc,nullptr));
		m_mapNetTcpDataPkgPtr.insert(pair<string,NetTcpDatPkgShrPtr>(strKey,move(ptr)));
	}
	else if(!bStatus)
	{
		iter->second.reset();
		m_mapNetTcpDataPkgPtr.erase(iter);
	}
}

//tcp服务器收到信息回调
void CTcpTransferMng::OnTcpMessage(const string & strPeerConnKey, const string & strLocalConnKey, const char * pData, const int nDataLen)
{
	//消息解包
	string&& strTcpServerPort = strLocalConnKey.substr(strLocalConnKey.find(':') + 1);
	string&& strKey = strTcpServerPort + strPeerConnKey;

	auto iter = m_mapNetTcpDataPkgPtr.find(strKey);
	if(m_mapNetTcpDataPkgPtr.end() != iter)
	{
		m_mapNetTcpDataPkgPtr[strKey]->UnDataPkg(pData,nDataLen);
	}
}

//tcp协议分发
void CTcpTransferMng:: DistributProtocolData(unsigned int u32Cmd, int nResult, const char* pBodyData, int s32BodyLen, const string& strTcpServerPort, const string& strPeerConnKey)
{
	//发布消息
	string&& strTopic = TcpSrvRecMsgTopic(strTcpServerPort);
	SCMsgBusMng.GetMsgBus()->SendReq<void, const string&, const string&,const unsigned int&,const char*&, const int&>(strTcpServerPort,strPeerConnKey,u32Cmd,pBodyData, s32BodyLen, strTopic);
}

//主动向tcp客户端发送数据
bool CTcpTransferMng::SendTcpData(const int & s32TcpServerPort,const string & strConnKey, const int& s32Cmd,const string& strData,const int& s32SendType)
{
	//查找解分包器
	string&& strKey = to_string(s32TcpServerPort) + strConnKey;
	auto iter = m_mapNetTcpDataPkgPtr.find(strKey);
	if(m_mapNetTcpDataPkgPtr.end() == iter)
	{
		return false;
	}

	//先封包后发送
	unsigned int nOutLen = 0;
	char pkgdata[nMaxPkgLen] = { 0 };
	iter->second->PkgProtocolData(s32SendType,s32Cmd,strData.c_str(),strData.length(),(char*)pkgdata,nOutLen,nMaxPkgLen);
	return m_pTcpServiceModule->SendData(strConnKey, string(pkgdata,nOutLen), s32TcpServerPort);
}

//主动关闭tcp客户端连接
void CTcpTransferMng::CloseTcpConnect(const string & strConnKey, const int & s32TcpServerPort)
{
	return m_pTcpServiceModule->CloseConnect(strConnKey, s32TcpServerPort);
}