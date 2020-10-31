#include "RPCClient.h"
#include "log4cxx/Loging.h"
#include "comm/CommFun.h"
#include "time/CpuExeTime.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace rpc_client;
using namespace common_cmmobj;
using namespace common_template;

#define RPC_CONNECT_TIMEOUT (2000)
#define RPC_REC_TIMEOUT     (10000)
#define RPC_SENT_TIMEOUT    (10000)

CRPCClient::CRPCClient():
m_bBusy(false),
m_bConn(false)
{
}

CRPCClient::~CRPCClient()
{
	CloseRPCConn(true);
}

bool CRPCClient::CreateClientService(const std::string& strIp, const int& s32Port)
{
	TSocketPtr socket = TSocketPtr(new TSocket(strIp.c_str(), s32Port));
	if (NULL == socket)
	{
		LOG_WARN("systerm") << "create RPC socket fail....";
		return false;
	}

	socket->setConnTimeout(RPC_CONNECT_TIMEOUT);
	socket->setRecvTimeout(RPC_REC_TIMEOUT);
	socket->setSendTimeout(RPC_SENT_TIMEOUT);

	m_TransportShrPtr = TTransportPtr(socket);
	if (!m_TransportShrPtr)
	{
		LOG_WARN("systerm") << "create RPC transport fail....";
		return false;
	}

	TBinaryProtocolPtr protocol = TBinaryProtocolPtr(new TBinaryProtocol(m_TransportShrPtr));
	if (!protocol)
	{
		LOG_WARN("systerm") << "create RPC binary protocol fail....";
		return false;
	}

	TMultiplexedProtocolPtr mprotocol = TMultiplexedProtocolPtr(new TMultiplexedProtocol(protocol, "com.bsb.slb.thrift.AccessService"));
	if (NULL == mprotocol)
	{
		LOG_WARN("systerm") << "create RPC mmprotocol fail....";
		return false;
	}

	m_ServiceClientShrPtr = RPC_ServiceClientPtr(new AccessServiceClient(mprotocol));
	if (!m_ServiceClientShrPtr)
	{
		LOG_WARN("systerm") << "create RPC servcie client fail...";
		return false;
	}

	SetWorkStatus(true);

	bool bRet = OpenRPCConn();
	SetConnStatus(bRet);
	
	return bRet;
}

bool CRPCClient::GetWorkStatus()
{
	CLock lock(&m_WorkStatusMutex);
	return m_bBusy;
}

void CRPCClient::SetWorkStatus(bool bStatus/*bStatus == false*/)
{
	CLock lock(&m_WorkStatusMutex);
	m_bBusy = bStatus;
}

bool CRPCClient::GetConnStatus()
{
	CLock lock(&m_ConnStatusMutex);
	return m_bConn;
}

void CRPCClient::SetConnStatus(bool bStatus/*bStatus == false*/)
{
	CLock lock(&m_ConnStatusMutex);
	m_bConn = bStatus;
}

bool CRPCClient::GetRpcService(RPC_ServiceClientPtr & ptrService)
{
	ptrService = m_ServiceClientShrPtr;
	return true;
}

bool CRPCClient::RpcCall(IF_RPC_SERVICE::Func&& fun, const std::string& strName)
{
	if (!OpenRPCConn())
	{
		SetWorkStatus();
		return false;
	}

	bool bRet = true;
	try
	{
		fun();
		LOG_DEBUG("systerm") << string_format("RpcCall|Func(%s) Exe Successful!\n", strName.c_str());
	}
	catch (TException &tx)
	{
		bRet = false;
		LOG_ERROR("systerm") << string_format("RpcCall|Func(%s) Exe ERROR: %s\n", strName.c_str(),tx.what()).c_str();

		CloseRPCConn();
	}	
	
	SetWorkStatus();

	return bRet;
}

bool CRPCClient::OpenRPCConn()
{
	if (GetConnStatus())
	{
		return true;
	}

	try
	{
		m_TransportShrPtr->open();
	}
	catch (TException &tx)
	{
		LOG_DEBUG("systerm") << string_format("OpenRPCConnect|Build RPC Connection Fail ERROR: %s\n", tx.what());
		return false;
	}
	LOG_DEBUG("systerm") << "OpenRPCConnect|Build RPC Connection Successful";

	SetConnStatus(true);

	return true;
}

void CRPCClient::CloseRPCConn(bool bRelease/*bRelease = false*/)
{
	m_TransportShrPtr->close();
	if (bRelease)
	{
		LOG_DEBUG("systerm") << "RpcClient Release Close Connect!\n";
	}
	else
	{
		LOG_DEBUG("systerm") << "Rpc Call Exception or First Open Close!\n";
	}

	SetConnStatus();
}