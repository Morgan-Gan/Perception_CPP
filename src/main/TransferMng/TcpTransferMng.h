#include <string>
#include <vector>
#include <map>
#include <functional>
#include "ITcpServiceModule.h"
#include "thread/Thread.h"
#include "comm/Singleton.h"
#include "CfgMng/CfgMng.h"
#include "network/NetTcpDataPkg.h"
#include "comm/CommDefine.h"

namespace MAIN_MNG
{
    class CTcpTransferMng : public ICTcpServiceCallback,
    public common_template::CSingleton<CTcpTransferMng>
    {
        friend class common_template::CSingleton<CTcpTransferMng>;
        using ThreadShrPtr = std::shared_ptr<common_cmmobj::CThread>;
        using NetTcpDatPkgShrPtr = std::shared_ptr<common_cmmobj::CNetTcpDataPkg>;
		using NetTcpDataPtrMap = std::map<std::string,NetTcpDatPkgShrPtr>;

    public:
        bool StartTcpSvr();

    	virtual void OnTcpConnect(const std::string& strPeerConnKey, const std::string& strLocalConnKey, bool bStatus);
		virtual void OnTcpMessage(const std::string& strConnKey, const std::string& strLocalConnKey, const char* pData, const int nDataLen);

    private:

        CTcpTransferMng() = default;
        ~CTcpTransferMng() = default;

        //tcp service消息订阅
		void TcpServerAttach(nlohmann::json&& elm, std::vector<std::tuple<unsigned short, int>>& vecServerCfg);

        //协议数据分发
        void DistributProtocolData(unsigned int cmd, int nResult, const char* pBodyData, int s32BodyLen, const std::string& strTcpServerPort, const std::string& strPeerConnKey);

		//发送客户数据
		bool SendTcpData(const int& s32TcpServerPort,const std::string& strConnKey, const int& s32Cmd,const std::string& strData,const int& s32SendType);
		
		//关闭客户端连接
		void CloseTcpConnect(const std::string& strConnKey, const int& s32TcpServerPort);

    private:
        ICTcpServiceModule* m_pTcpServiceModule;
        
		NetTcpDataPtrMap m_mapNetTcpDataPkgPtr;
    };
    #define SCTcpTransferMng (common_template::CSingleton<CTcpTransferMng>::GetInstance())
}