#include <algorithm>
#include "RPCClientMng.h"
#include "comm/ReleaseThread.h"
#include "IRpcServiceModule.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace rpc_client;
using namespace common_cmmobj;
using namespace common_template;

const int nDefaultRPCConnectCount = 100;
const int nDefaultAppPort = 47390;
const char* pDefaultAppIp = "127.0.0.1";
const int s32ThreadCycle = 5;
const int s32AliveCycle = 60;
const int s32IDStrLen = 9;

#define RPC_CONNECT_TIMEOUT (2000)
#define RPC_REC_TIMEOUT     (10000)
#define RPC_SENT_TIMEOUT    (10000)

extern "C" bool IF_RPC_SERVICE::GetRpc(Any&& RpcClient, Any&& RpcService)
{
	return SCRPCManager.GetRpc(std::move(RpcClient), std::move(RpcService));
}

extern "C" bool IF_RPC_SERVICE::CallRpcFunc(IF_RPC_SERVICE::Func&& func, const std::string& strFuncName, Any&& AnyClientObj)
{
	return SCRPCManager.CallRpcFunc(std::move(func), strFuncName, std::move(AnyClientObj));
}

extern "C" void IF_RPC_SERVICE::RpcInit(const char* pAppIp, int nApport, int nMaxRpcConn)
{
	SCRPCManager.Init(pAppIp, nApport, nMaxRpcConn);
}

CRPCClientMng::CRPCClientMng()
{
	m_bInited = false;
}

CRPCClientMng::~CRPCClientMng()
{
}

void CRPCClientMng::Init(const char* pAppIp, int nApport, int nMaxRpcConn)
{
	m_strAppServerIp = (0 == strlen(pAppIp)) ? pDefaultAppIp : pAppIp;
	m_nAppServerPort = (0 >= nApport) ? nDefaultAppPort : nApport;
	m_nMaxRpcConn = (0 >= nMaxRpcConn) ? nDefaultRPCConnectCount : nMaxRpcConn;
	m_bInited = true;
}

bool CRPCClientMng::CallRpcFunc(IF_RPC_SERVICE::Func && fun, const std::string & strFunName, Any&& AnyObj)
{
	if (!m_bInited)
	{
		return false;
	}

	return AnyObj.AnyCast<RPCClientShrPtr>()->RpcCall(std::move(fun), strFunName);
}

bool CRPCClientMng::GetRpc(Any&& RpcClient, Any&& RpcService)
{
	RPC_ServiceClientPtr ptrService;
	RPCClientShrPtr ptrRPCClient;
	if (!GetClient(ptrService, ptrRPCClient))
	{
		return false;
	}

	RpcClient = ptrRPCClient;
	RpcService = ptrService;

	return true;
}

bool CRPCClientMng::GetClient(RPC_ServiceClientPtr& ptrService, RPCClientShrPtr & ptrRPCClient)
{
	if (!GetRpcClientFromPool(ptrRPCClient))
	{
		return false;
	}

	return ptrRPCClient->GetRpcService(ptrService);
}

bool CRPCClientMng::GetRpcClientFromPool(RPCClientShrPtr & ptrRPCClient)
{
	CLock lock(&m_Mutex);
	for (RPCClientShrPtr& rpcClientObjPtr : m_vecRpcClientObj)
	{
		if (!rpcClientObjPtr->GetWorkStatus())
		{
			ptrRPCClient = rpcClientObjPtr;
			rpcClientObjPtr->SetWorkStatus(true);
			return true;
		}
	}

	RPCClientShrPtr ptrTemp = RPCClientShrPtr(new CRPCClient);
	if (!ptrTemp->CreateClientService(m_strAppServerIp, m_nAppServerPort))
	{
		return false;
	}
	ptrRPCClient = ptrTemp;

	if (m_nMaxRpcConn > m_vecRpcClientObj.size())
	{
		m_vecRpcClientObj.push_back(ptrRPCClient);
		LOG_DEBUG("systerm") << "create a new long conn rpc client successful\n";
	}
	else
	{
		LOG_DEBUG("systerm") << "create a new short conn rpc client successful\n";
	}

	return true;
}