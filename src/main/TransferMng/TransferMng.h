#pragma once
#include <vector>
#include <tuple>
#include <string>
#include "comm/Singleton.h"
#include "comm/Any.h"
#include "thread/Thread.h"
#include "ITcpServiceModule.h"
#include "IRpcServiceModule.h"
#include "IVideoServiceModule.h"
#include "CfgMng/CfgMng.h"
#include "rpc/RpcWrap.h"
namespace MAIN_MNG
{
	class CTransferMng : public ICTcpServiceCallback,public common_template::CSingleton<CTransferMng>
	{
		friend class common_template::CSingleton<CTransferMng>;
		using ThreadShrPtr = std::shared_ptr<common_cmmobj::CThread>;
		using RPCServicePtr = boost::shared_ptr<common_template::AccessServiceClient>;

	public:
		//北向通信启动
		bool StartNorthTrans();

		//南向通信启动
		bool StartSouthTrans();

	public:
		virtual void OnTcpConnect(const std::string& strPeerConnKey, const std::string& strLocalConnKey, bool bStatus);
		virtual void OnTcpMessage(const std::string& strConnKey, const std::string& strLocalConnKey, const char* pData, const int nDataLen);

	private:
		/*南向通信*/
		/**serial**/
		void StartSerial();

		/**rtsp cli**/
		void StartRtspCli();

		/**tcp server**/
		bool StartTcpServer();

		//tcp service消息订阅
		void TcpServerAttach(nlohmann::json&& elm, std::vector<std::tuple<unsigned short, int>>& vecServerCfg);

		//发送客户数据
		bool SendTcpData(const std::string& strConnKey, const std::string& strData, const int& s32TcpServerPort);
		
		//关闭客户端连接
		void CloseTcpConnect(const std::string& strConnKey, const int& s32TcpServerPort);

		/*北向通信*/
		/**rpc cli **/
		bool StartRpcli();

		bool Online();

		void AliveLoop();

		bool SendRpcData(IF_RPC_SERVICE::Func&& func, common_template::Any&& AnyRpcClient, const std::string& strFuncName);

		bool GetRpcService(common_template::Any&& AnyRpcClient, common_template::Any&& AnyRpcService);
	
		/*http服务*/
		/**http cli**/
		void StartHttpCli();

	private:
		CTransferMng();
		~CTransferMng();

	private:
		ICTcpServiceModule* m_pTcpServiceModule;
		ThreadShrPtr m_ptrOnlineThread;

		//rpc
		std::string m_strRpcServiceTopic;
		std::string m_strRpcSendDataTopic;
		std::string m_strAccessId;
		std::string m_strIpAddress;
		std::string m_strMacAddress;
	};

#define SCTransferMng (common_template::CSingleton<CTransferMng>::GetInstance())
}