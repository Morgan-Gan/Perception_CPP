#pragma once
#include <string>

//transfer 
////cfg
const std::string strTransferNode("Transfer");
const std::string strTransferSouthNode("South");
const std::string strTransferNorthNode("North");
const std::string strTcpSrvType("tcpsrv");
const std::string strTcpCliType("tcpcli");
const std::string strRpcType("rpcli");
const std::string strHttpCliType("httpcli");
const std::string strRtspCliType("rtspcli");
const std::string strSerialType("serival");

////rpc
const std::string strGetRpcService("GetRpcService");
const std::string strSendRpcData("SendRpcData");
static std::string GetRpcServiceTopic(const std::string& strIp, const std::string& strPort)
{
	return std::string(strRpcType + strIp + strPort + strGetRpcService);
}

static std::string GetRpcSendTopic(const std::string& strIp, const std::string& strPort)
{
	return std::string(strRpcType + strIp + strPort + strSendRpcData);
}

////tcpserver
const std::string strRevConn("tcpcli_conn");
const std::string strSendConn("tcpcli_close");
const std::string strRevMsg("tcpcli_revmsg");
const std::string strSendMsg("tcpcli_sendmsg");

static std::string GetTcpCliConnTopic(const std::string& strPort)
{
	return std::string(strTcpSrvType + strPort + strRevConn);
}

static std::string GetTcpCliCloseTopic(const std::string& strPort)
{
	return std::string(strTcpSrvType + strPort + strSendConn);
}

static std::string GetTcpSrvRecMsgTopic(const std::string& strPort)
{
	return std::string(strTcpSrvType + strPort + strRevMsg);
}

static std::string GetTcpCliSendMsgTopic(const std::string& strPort)
{
	return std::string(strTcpSrvType + strPort + strSendMsg);
}

//db
////cfg
const std::string strDbNode("Db");
const std::string strDbRemoteNode("Remote");
const std::string strDbLocateNode("Locate");
const std::string strRedisCliType("rediscli");

////redis
const std::string strSaveRedis("SaveToRedis");
const std::string strGetRedis("GetFromRedis");
const std::string strDelRedis("DelFromRedis");

static std::string GetRedisSaveTopic(const std::string& strIp, const std::string& strPort)
{
	return std::string(strRedisCliType + strIp + strPort + strSaveRedis);
}

static std::string GetRedisReadTopic(const std::string& strIp, const std::string& strPort)
{
	return std::string(strRedisCliType + strIp + strPort + strGetRedis);
}

static std::string GetRedisDelTopic(const std::string& strIp, const std::string& strPort)
{
	return std::string(strRedisCliType + strIp + strPort + strDelRedis);
}

//Alg
////cfg
const std::string strAlgNode("Alg");
const std::string strAlgDetectNode("Detect");
const std::string strAlgActionRecgNode("ActionRecg");
const std::string strAlgDetectYolo("DetectYolo");
const std::string strAlgDetectSsd("DetectSsd");