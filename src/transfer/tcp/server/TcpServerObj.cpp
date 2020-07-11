#include "TcpServerObj.h"
#include "TcpServiceModule.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace muduotcp;

CTcpServerObj::CTcpServerObj(EventLoop* loop, const InetAddress& listenAddr) :
	server_(loop, listenAddr, "TcpServerObj"),
	codec_(bind(&CTcpServerObj::onStringMessage, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4))
{
	server_.setConnectionCallback(bind(&CTcpServerObj::onConnection, this, placeholders::_1));
	server_.setMessageCallback(bind(&LengthHeaderCodec::onMessage, &codec_, placeholders::_1, placeholders::_2, placeholders::_3));
}

void CTcpServerObj::setThreadNum(int numThreads)
{
	server_.setThreadNum(numThreads);
}

void CTcpServerObj::start()
{
	server_.start();
}

string CTcpServerObj::GetPeerConnectStr(const TcpConnectionPtr& conn)
{
	string strKey(conn->peerAddress().toIpPort().c_str());
	return strKey;
}

string CTcpServerObj::GetLocalConnectStr(const TcpConnectionPtr& conn)
{
	string strKey(conn->localAddress().toIpPort().c_str());
	return strKey;
}

TcpConnectionPtr CTcpServerObj::GetTcpConnectPtr(const string& strConnKey)
{
	MutexLockGuard lock(mutex_);
	weak_ptr<TcpConnection> weakptr;
	weakptr = LocalConnectionsMap[strConnKey];
	TcpConnectionPtr conn = weakptr.lock();
	return conn;
}

void CTcpServerObj::AddOrDelTcpConnectPtr(const TcpConnectionPtr& conn, const string& strConnKey, bool bStatus)
{
	MutexLockGuard lock(mutex_);
	if (bStatus)
	{
		LocalConnectionsMap[strConnKey] = conn;
	}
	else
	{
		LocalConnectionsMap[strConnKey].reset();
		LocalConnectionsMap.erase(strConnKey);
	}
}

void CTcpServerObj::onConnection(const TcpConnectionPtr& conn)
{
	bool bRet = conn->connected();
	string strPeerConnKey = GetPeerConnectStr(conn);
	string strLocalConnKey = GetLocalConnectStr(conn);
	AddOrDelTcpConnectPtr(conn, strPeerConnKey, bRet);

	CTcpServiceModule::GetInstance()->OnConnect(strPeerConnKey, strLocalConnKey, bRet);
}

void CTcpServerObj::onStringMessage(const TcpConnectionPtr& conn, const char* message, int32_t len, Timestamp)
{
	string strPeerConnKey = GetPeerConnectStr(conn);
	string strLocalConnKey = GetLocalConnectStr(conn);

	CTcpServiceModule::GetInstance()->OnMessage(strPeerConnKey, strLocalConnKey, message, len);
}

void CTcpServerObj::sendData(const string& strConnKey, const char* data, const int nDataLen)
{
	TcpConnectionPtr conn = GetTcpConnectPtr(strConnKey);
	if (conn)
	{
		codec_.send(get_pointer(conn), data, nDataLen);
	}
}

void CTcpServerObj::closeConnect(const string& strConnKey)
{
	TcpConnectionPtr conn = GetTcpConnectPtr(strConnKey);
	if (conn)
	{
		codec_.closeConnect(get_pointer(conn));
	}
}