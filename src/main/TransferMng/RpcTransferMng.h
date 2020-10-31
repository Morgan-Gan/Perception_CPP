#include <string>
#include <vector>
#include <map>
#include <functional>
#include "IRpcServiceModule.h"
#include "comm/Singleton.h"
#include "CfgMng/CfgMng.h"
#include "thread/Thread.h"
#include "comm/CommDefine.h"
#include "rpc/RpcWrap.h"

namespace MAIN_MNG
{
    class CRpcTransferMng : public common_template::CSingleton<CRpcTransferMng>
    {
        friend class common_template::CSingleton<CRpcTransferMng>;
        using ThreadShrPtr = std::shared_ptr<common_cmmobj::CThread>;

    public:
        bool StartRpcli();

    private:
        CRpcTransferMng();
        ~CRpcTransferMng() = default;

        bool Online();

		void AliveLoop();

		bool SendRpcData(IF_RPC_SERVICE::Func&& func, common_template::Any&& AnyRpcClient, const std::string& strFuncName);

		bool GetRpcService(common_template::Any&& AnyRpcClient, common_template::Any&& AnyRpcService);

    private:
        //心跳线程
    	ThreadShrPtr m_ptrOnlineThread;

		//rpc
		std::string m_strRpcServiceTopic;
		std::string m_strRpcSendDataTopic;
		std::string m_strAccessId;
		std::string m_strIpAddress;
		std::string m_strMacAddress;
     
    };
    #define SCRpcTransferMng (common_template::CSingleton<CRpcTransferMng>::GetInstance())
}