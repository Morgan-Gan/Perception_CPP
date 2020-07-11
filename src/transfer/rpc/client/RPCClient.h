#pragma once
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <string>
#include "gencpp/AccessService.h"
#include "IRpcServiceModule.h"
#include "mutex/LibMutex.h"

namespace rpc_client
{
	using RPC_ServiceClientPtr = boost::shared_ptr<AccessServiceClient>;
	using TTransportPtr = boost::shared_ptr<apache::thrift::transport::TTransport>;
	using TBinaryProtocolPtr = boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>;
	using TMultiplexedProtocolPtr = boost::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol>;
	using TSocketPtr = boost::shared_ptr<apache::thrift::transport::TSocket>;

	class CRPCClient
	{
		using CMutexType = common_cmmobj::CMutex;
	public:
		CRPCClient();
		~CRPCClient();
		bool CreateClientService(const std::string& strIp, const int& s32Port);
		bool GetWorkStatus();
		void SetWorkStatus(bool bStatus = false);
		bool GetConnStatus();
		void SetConnStatus(bool bStatus = false);
		bool GetRpcService(RPC_ServiceClientPtr& ptrService);
		bool RpcCall(IF_RPC_SERVICE::Func&& fun, const std::string& strName);

	private:
		bool OpenRPCConn();
		void CloseRPCConn(bool bRelease = false);

	private:
		TTransportPtr m_TransportShrPtr;
		RPC_ServiceClientPtr m_ServiceClientShrPtr;

		CMutexType m_WorkStatusMutex;
		bool m_bBusy;

		CMutexType m_ConnStatusMutex;
		bool m_bConn;
	};
}