#pragma once
#include <vector>
#include <tuple>
#include <string>
#include "comm/Singleton.h"
#include "comm/Any.h"
#include "thread/Thread.h"

namespace MAIN_MNG
{
	class CTransferMng : public common_template::CSingleton<CTransferMng>
	{
		friend class common_template::CSingleton<CTransferMng>;
		using ThreadShrPtr = std::shared_ptr<common_cmmobj::CThread>;

	public:
		//北向通信启动
		bool StartNorthTrans();

		//南向通信启动
		bool StartSouthTrans();

	private:
		/*南向通信*/
		/**serial**/
		void StartSerial();

		/**rtsp cli**/
		void StartRtspCli();

		/**tcp server**/
		bool StartTcpServer();

		/*北向通信*/
		/**rpc cli **/
		bool StartRpcli();
	
		/*http服务*/
		/**http cli**/
		void StartHttpCli();

	private:
		CTransferMng() = default;
		~CTransferMng() = default;

	};

#define SCTransferMng (common_template::CSingleton<CTransferMng>::GetInstance())
}