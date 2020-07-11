#pragma once
#include "codec.h"
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <set>
#include <map>
#include <string>

typedef std::map<std::string,muduo::net::TcpConnectionPtr> ConnectionMap;

namespace muduotcp
{
class CTcpServerObj
{
public:
  CTcpServerObj(muduo::net::EventLoop* loop,const muduo::net::InetAddress& listenAddr);
  void setThreadNum(int numThreads);
  void start();
  void sendData(const std::string& strConnKey,const char* data,const int nDataLen);
  void closeConnect(const std::string& strConnKey);

private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn);
  void onStringMessage(const muduo::net::TcpConnectionPtr&,const char* message,int32_t len,muduo::Timestamp);
  muduo::net::TcpConnectionPtr GetTcpConnectPtr(const std::string& strConnKey);
  void AddOrDelTcpConnectPtr(const muduo::net::TcpConnectionPtr& ptr,const std::string& strConnKey,bool bStatus);
  std::string GetPeerConnectStr(const muduo::net::TcpConnectionPtr& conn);
  std::string GetLocalConnectStr(const muduo::net::TcpConnectionPtr& conn);

private:
  muduo::net::TcpServer server_;
  muduotcp::LengthHeaderCodec codec_;
  
  muduo::MutexLock mutex_;
  ConnectionMap LocalConnectionsMap;
};
}
