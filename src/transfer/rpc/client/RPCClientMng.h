#pragma once
#include <vector>
#include <list>
#include <map>
#include <atomic>
#include "RPCClient.h"
#include "comm/Singleton.h"
#include "comm/Any.h"

namespace rpc_client
{
	class CRPCClientMng : public common_template::CSingleton<CRPCClientMng>
	{
		friend class common_template::CSingleton<CRPCClientMng>;
		using RPCClientShrPtr = boost::shared_ptr<CRPCClient>;

	public:
		void Init(const char* pAppIp, int nApport, int nMaxRpcConn);
		bool GetRpc(common_template::Any&& RpcClient, common_template::Any&& RpcService);
		bool CallRpcFunc(IF_RPC_SERVICE::Func && fun, const std::string & strFunName, common_template::Any&& AnyObj);

	private:
		CRPCClientMng();
		virtual ~CRPCClientMng();

		bool GetRpcClientFromPool(RPCClientShrPtr& ptrRRCClient);
		bool GetClient(RPC_ServiceClientPtr& ptrClient, RPCClientShrPtr& ptrRRCClient);

	private:
		std::string m_strAppServerIp;
		std::string m_strDiscernSysCode;
		int m_nAppServerPort;
		int m_nMaxRpcConn;

		common_cmmobj::CMutex m_Mutex;
		std::vector<RPCClientShrPtr> m_vecRpcClientObj;

		std::atomic_bool m_bInited;
	};

#define  SCRPCManager (common_template::CSingleton<CRPCClientMng>::GetInstance())
}
