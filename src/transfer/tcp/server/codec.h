#pragma once
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <functional>
#include <memory>

namespace muduotcp
{
class LengthHeaderCodec
{
public:
  typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const char* message,
								                int32_t len,
                                muduo::Timestamp)> MessageCallback;

  explicit LengthHeaderCodec(const MessageCallback& cb)
    : messageCallback_(cb)
  {
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime)
  {
	while(buf->readableBytes() > 0)
	{
		int len = buf->readableBytes();
        messageCallback_(conn,buf->peek(),len,receiveTime);
        buf->retrieve(len);
	}
  }

  //TcpConnectionPtr
  void send(muduo::net::TcpConnection* conn,const char* message,const int nDataLen)
  {
    muduo::net::Buffer buf;
    buf.append(message,nDataLen);
    conn->send(&buf);
  }

  //close tcp client connect
  void closeConnect(muduo::net::TcpConnection* conn)
  {
	  conn->forceClose();
  }

private:
  MessageCallback messageCallback_;
};
}