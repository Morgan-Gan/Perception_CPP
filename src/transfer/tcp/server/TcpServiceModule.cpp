#include "TcpServiceModule.h"
#include "boost/BoostFun.h"
#include "TcpServerObj.h"

using namespace std;
using namespace muduotcp;

extern "C" ICTcpServiceModule* GetTCPServiceModuleInstance()
{
	return CTcpServiceModule::GetInstance();
}

CTcpServiceModule::CTcpServiceModule() :
	m_pCallbackObj(0)
{
}

CTcpServiceModule::~CTcpServiceModule()
{
}

CTcpServiceModule* CTcpServiceModule::GetInstance()
{
	static CTcpServiceModule* pInstance = new CTcpServiceModule();
	return pInstance;
}

void CTcpServiceModule::SetCallbackObj(ICTcpServiceCallback* pCallbackObj)
{
	if (!m_pCallbackObj)
	{
		m_pCallbackObj = pCallbackObj;
	}
}

bool CTcpServiceModule::StartTCPServer(vector<tuple<unsigned short, int>>&& vecServerCfg)
{
	muduo::net::EventLoop loop;
	CTcpServerObj* pGwTcpServer = nullptr;
	for (auto val : vecServerCfg)
	{
		unsigned short u16Port;
		int s32Num;
		std::tie(u16Port, s32Num) = val;
		muduo::net::InetAddress selfGwServerAddr(u16Port);
		pGwTcpServer = new CTcpServerObj(&loop, selfGwServerAddr);
		pGwTcpServer->setThreadNum(s32Num);
		pGwTcpServer->start();
		m_mapTcpServer[u16Port] = pGwTcpServer;
	}
	loop.loop();
	return true;
}

void CTcpServiceModule::OnConnect(const std::string& strPeerConnKey, const std::string& strLocalConnKey, bool bStatus)
{
	if (m_pCallbackObj)
	{
		m_pCallbackObj->OnTcpConnect(strPeerConnKey, strLocalConnKey, bStatus);
	}
}

void CTcpServiceModule::OnMessage(const std::string& strPeerConnKey, const std::string& strLocalConnKey, const char* pData, const int nDataLen)
{
	if (m_pCallbackObj)
	{
		m_pCallbackObj->OnTcpMessage(strPeerConnKey, strLocalConnKey, pData, nDataLen);
	}
}

bool CTcpServiceModule::SendData(const std::string& strConnKey, const std::string& strData, const int& s32TcpServerPort)
{
	if (strData.empty())
	{
		return false;
	}

	auto ite = m_mapTcpServer.find(s32TcpServerPort);
	if (ite != m_mapTcpServer.end())
	{
		m_mapTcpServer[s32TcpServerPort]->sendData(strConnKey, strData.c_str(), strData.length());
	}

	return true;
}

void CTcpServiceModule::CloseConnect(const std::string& strConnKey, const int& s32TcpServerPort)
{
	auto ite = m_mapTcpServer.find(s32TcpServerPort);
	if (ite != m_mapTcpServer.end())
	{
		m_mapTcpServer[s32TcpServerPort]->closeConnect(strConnKey);
	}
}