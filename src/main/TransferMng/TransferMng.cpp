#include "TransferMng.h"
#include "CfgMng/CfgMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "TcpTransferMng.h"
#include "RpcTransferMng.h"
#include "VsTransferMng.h"
#include "log4cxx/Loging.h"
#include "SubTopic.h"

using namespace std;
using namespace MAIN_MNG;
using namespace common_template;
using namespace common_cmmobj;

/*南向通信启动*/
bool CTransferMng::StartSouthTrans()
{
	//启动串口通信
	StartSerial();

	//启动视频拉流通信
	StartRtspCli();

	//启动tcp服务器通信
	return StartTcpServer();
}

//串口通信
void CTransferMng::StartSerial()
{
}

//rstp视频拉流客户端通信
void CTransferMng::StartRtspCli()
{
	SCVsTransferMng.StartCameras();
}

//tcp服务器通信
bool CTransferMng::StartTcpServer()
{
	//启动tcp服务器
	return SCTcpTransferMng.StartTcpSvr();
}

/*北向通信启动*/
bool CTransferMng::StartNorthTrans()
{
	//启动http客户端通信
	StartHttpCli();

	//启动rpc客户端通信
	return StartRpcli();
}

//启动http通信(客户端)
void CTransferMng::StartHttpCli()
{
}

//启动rpc通信(客户端)
bool CTransferMng::StartRpcli()
{
	return SCRpcTransferMng.StartRpcli();
}
