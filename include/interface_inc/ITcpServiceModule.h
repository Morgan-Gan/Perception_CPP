#pragma once
#include <string>
#include <vector>
#include <tuple>

class ICTcpServiceCallback
{
public:
	virtual void OnTcpConnect(const std::string& strPeerConnKey, const std::string& strLocalConnKey, bool bStatus) = 0;
	virtual void OnTcpMessage(const std::string& strConnKey, const std::string& strLocalConnKey, const char* pData, const int nDataLen) = 0;
};

class ICTcpServiceModule
{
public:
	virtual bool StartTCPServer(std::vector<std::tuple<unsigned short,int>>&& vecServerCfg) = 0;
	virtual void SetCallbackObj(ICTcpServiceCallback* pCallbackObj) = 0;
	virtual bool SendData(const std::string& strConnKey, const std::string& strData, const int& s32TcpServerPort) = 0;
	virtual void CloseConnect(const std::string& strConnKey, const int& s32TcpServerPort) = 0;
};

extern "C" ICTcpServiceModule* GetTCPServiceModuleInstance();
