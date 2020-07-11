#pragma once
#include <vector>
#include <string>
#include "comm/Singleton.h"
#include "msgbus/MessageBus.h"
#include "nlohmann/json.hpp"
#include "SubTopic.h"

namespace common_cmmobj
{
	class CRedisOpMng
	{
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;

	public:
		CRedisOpMng() {}
		~CRedisOpMng() {}

		inline void InitCfg(MsgBusShrPtr ptrMsgBus, nlohmann::json&& RdJson)
		{
			if (!m_ptrMsgBus)
			{
				m_ptrMsgBus = ptrMsgBus;
			}

			//��ȡ�������ݵĹؼ���
			std::string&& strType = RdJson["type"];
			std::string&& strIp = RdJson["ip"];
			std::string&& strPort = RdJson["port"];
			m_strSaveDbTopic = GetRedisSaveTopic(strIp, strPort);
			m_strGetDbTopic = GetRedisReadTopic(strIp, strPort);
			m_strDelDbTopic = GetRedisDelTopic(strIp, strPort);
		}

		inline bool SaveRedisData(const std::string& strKey, const std::string& strData)
		{
			if (!m_ptrMsgBus || m_strSaveDbTopic.empty())
			{
				return false;
			}

			m_ptrMsgBus->SendReq<bool, const std::string&, const std::string&>(strKey, strData, m_strSaveDbTopic);
			return true;
		}

		inline void GetRedisData(const std::string& strKey, std::vector<std::string>& vecData)
		{
			if (!m_ptrMsgBus || m_strGetDbTopic.empty())
			{
				return;
			}

			m_ptrMsgBus->SendReq<void, const std::string&, std::vector<std::string>&>(strKey, vecData, m_strGetDbTopic);
		}

		inline void DelRedisData(const std::string& strKey)
		{
			if (!m_ptrMsgBus || m_strDelDbTopic.empty())
			{
				return;
			}

			m_ptrMsgBus->SendReq<void, const std::string&>(strKey, m_strDelDbTopic);
		}

	private:
		MsgBusShrPtr m_ptrMsgBus;
		std::string m_strSaveDbTopic;
		std::string m_strGetDbTopic;
		std::string m_strDelDbTopic;
	};
}