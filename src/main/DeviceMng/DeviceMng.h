#pragma once
#include <string>
#include <vector>
#include "comm/Singleton.h"
#include "nlohmann/json.hpp"

namespace MAIN_MNG
{
	class CDeviceMng : public common_template::CSingleton<CDeviceMng>
	{
		friend class common_template::CSingleton<CDeviceMng>;
	public:
		bool LoadAllDevices();
		std::vector<nlohmann::json> GetSouthTcpTransCfg();
		std::vector<nlohmann::json> GetReportTransCfg();
		std::vector<nlohmann::json> GetSaveDbCfg();
		std::vector<nlohmann::json> GetCfgToolCfg();

	private:
		CDeviceMng();
		~CDeviceMng();

		bool InitDevice(const std::string& strSoName, nlohmann::json&& cfgObj);
	private:
		std::vector<nlohmann::json> m_vecSouthTcpTransCfg;
		std::vector<nlohmann::json> m_vecReportTransCfg;
		std::vector<nlohmann::json> m_vecSaveDbCfg;
		std::vector<nlohmann::json> m_vecCfgToolCfg;
	};

#define  SCDeviceMng (common_template::CSingleton<CDeviceMng>::GetInstance())
}
