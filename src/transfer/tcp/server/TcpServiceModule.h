#pragma once
#include <map>
#include "ITcpServiceModule.h"

namespace muduotcp
{
    class CTcpServerObj;
    class CTcpServiceModule : public ICTcpServiceModule
    {
    public:
        static CTcpServiceModule* GetInstance();

        virtual bool StartTCPServer(std::vector<std::tuple<unsigned short,int>>&& vecServerCfg);
        virtual void SetCallbackObj(ICTcpServiceCallback* pCallbackObj);
        virtual bool SendData(const std::string& strConnKey, const std::string& strData, const int& s32TcpServerPort);
        virtual void CloseConnect(const std::string& strConnKey, const int& s32TcpServerPort);

        void OnConnect(const std::string& strPeerConnKey, const std::string& strLocalConnKey, bool bStatus);
        void OnMessage(const std::string& strPeerConnKey, const std::string& strLocalConnKey, const char* pData, const int nDataLen);

    private:
        CTcpServiceModule();
        virtual ~CTcpServiceModule();

    private:
        ICTcpServiceCallback* m_pCallbackObj;
        std::map<int, CTcpServerObj*> m_mapTcpServer;
    };
} // namespace muduotcp

