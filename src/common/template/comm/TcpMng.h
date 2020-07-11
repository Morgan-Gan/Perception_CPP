#pragma once
#include <string>
#include "BoostFun.h"
#include "MessageBus.h"
#include "LibMutex.h"
#include "nlohmann/json.hpp"
#include "SubTopic.h"

namespace common_template
{
	template<class T>
	class CTcpMng
	{
	public:
		using ClientPtr = boost::shared_ptr<T>;
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
		using CliObjMap = std::map<std::string, ClientPtr>;

	public:
		void InitCfg(MsgBusShrPtr ptrMegBus, nlohmann::json&& AccessJson);
		bool SendTcpData(const std::string& strConnKey, const char* pData, const int nDataLen);
		void ForceCloseClientConnect();
		CliObjMap GetAllCli()
		{
			common_cmmobj::CLock lock(&m_Mutex);
			return CliObjMap(m_CliObjMap);
		}

	private:
		void AttachTcpData(const std::string&& strConnTopic, const std::string&& strRecvMsgTopic);
		void OnConnect(const std::string& strPeerConnKey, bool& bStatus);
		void OnMessage(const std::string& strPeerConnKey, const char* pData, const int nDataLen);
		void CloseTcpConn(const std::string& strConnKey);
		ClientPtr GetClient(const std::string& strConnKey);
		void AddOrDelCliMap(ClientPtr& pCliObj, const std::string& strConnKey, bool bStatus);

	public:
		CTcpMng() {}
		~CTcpMng() {}

	private:
		common_cmmobj::CMutex m_Mutex;
		CliObjMap m_CliObjMap;

		MsgBusShrPtr m_ptrMsgBus;
		std::string m_strServerPort;
		std::string m_strServerType;
	};

	template<class T>
	inline void CTcpMng<T>::InitCfg(MsgBusShrPtr ptrMegBus, nlohmann::json && AccessJson)
	{
		//������Ϣ���ߺ�����
		if (!m_ptrMsgBus)
		{
			m_ptrMsgBus = ptrMegBus;
		}

		//��������tcp������Ϣ
		std::string&& strType = AccessJson["type"];
		std::string&& strPort = AccessJson["port"];
		std::string strRecvConnTopic = GetTcpConnTopic(std::move(strPort));
		std::string strRecvMsgTopic = GetTcpRecMsgTopic(strPort);
		AttachTcpData(std::move(strRecvConnTopic), std::move(strRecvMsgTopic));
		m_strServerPort = strPort;
		m_strServerType = strType;
	}

	template<class T>
	inline bool CTcpMng<T>::SendTcpData(const std::string & strConnKey, const char * pData, const int nDataLen)
	{
		if (!m_ptrMsgBus)
		{
			return false;
		}

		std::string strData(pData, nDataLen);
		std::string strTopic = GetTcpSendMsgTopic(m_strServerPort);
		m_ptrMsgBus->SendReq<bool, const std::string&, const std::string&, const int&>(strConnKey, strData, atoi(m_strServerPort.c_str()), strTopic);
		return true;
	}

	template<class T>
	inline void CTcpMng<T>::AttachTcpData(const std::string&& strRecvConnTopic, const std::string&& strRecvMsgTopic)
	{
		//���Ļص���Ϣ
		m_ptrMsgBus->Attach([this](const std::string & strPeerConnKey, bool& bStatus) {OnConnect(strPeerConnKey, bStatus); }, strRecvConnTopic);

		m_ptrMsgBus->Attach([this](const std::string & strPeerConnKey, const char*& pData, const int& s32DataLen) {OnMessage(strPeerConnKey, pData, s32DataLen); }, strRecvMsgTopic);
	}

	template<class T>
	inline void CTcpMng<T>::OnConnect(const std::string & strPeerConnKey, bool & bStatus)
	{
		using ClientPtr = boost::shared_ptr<T>;
		ClientPtr pClientObj = GetClient(strPeerConnKey);
		if (!pClientObj)
		{
			ClientPtr pTemp(new T(strPeerConnKey));
			pClientObj = pTemp;
		}
		AddOrDelCliMap(pClientObj, strPeerConnKey, bStatus);
	}

	template<class T>
	inline void CTcpMng<T>::OnMessage(const std::string & strPeerConnKey, const char * pData, const int nDataLen)
	{
		using ClientPtr = boost::shared_ptr<T>;
		ClientPtr pClientObj = GetClient(strPeerConnKey);
		if (pClientObj)
		{
			pClientObj->MessageRecv(pData, nDataLen);
		}
	}

	template<class T>
	inline void CTcpMng<T>::ForceCloseClientConnect()
	{
		common_cmmobj::CLock lock(&m_Mutex);
		ClientPtr pClient;
		for (auto val : m_CliObjMap)
		{
			pClient = (ClientPtr)val.second;
			if (!pClient)
			{
				continue;
			}

			if (pClient->GetHeartbeatStatus())
			{
				CloseTcpConn(val.first);
			}
		}
	}

	template<class T>
	inline void CTcpMng<T>::CloseTcpConn(const std::string & strConnKey)
	{
		if (m_ptrMsgBus)
		{
			std::string strTopic = GetTcpCloseTopic(m_strServerPort);
			m_ptrMsgBus->SendReq<bool, const std::string&, const int &>(strConnKey, atoi(m_strServerPort.c_str()), strTopic);
		}
	}

	template<class T>
	inline boost::shared_ptr<T> CTcpMng<T>::GetClient(const std::string & strConnKey)
	{
		common_cmmobj::CLock lock(&m_Mutex);
		boost::weak_ptr<T> weakptr;
		weakptr = m_CliObjMap[strConnKey];

		ClientPtr pClientObj = weakptr.lock();
		return pClientObj;
	}

	template<class T>
	inline void CTcpMng<T>::AddOrDelCliMap(ClientPtr & pCliObj, const std::string & strConnKey, bool bStatus)
	{
		common_cmmobj::CLock lock(&m_Mutex);
		if (bStatus)
		{
			pCliObj->OnConnect();
			m_CliObjMap[strConnKey] = pCliObj;
		}
		else
		{
			pCliObj->OnClose();
			m_CliObjMap.erase(strConnKey);
		}
	}
}