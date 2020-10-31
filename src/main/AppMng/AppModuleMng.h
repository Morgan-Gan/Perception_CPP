#pragma once
#include <string>
#include <vector>
#include <map>
#include "comm/Singleton.h"
#include "comm/CommDefine.h"

namespace MAIN_MNG
{
	class CAppModuleMng : public common_template::CSingleton<CAppModuleMng>
	{
		friend class common_template::CSingleton<CAppModuleMng>;
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
		using MapDllType = std::map<std::string, DllShrPtr>;
		using TaskCfgType = std::tuple<Json,Json,vector<Json>>;
		using LstTaskCfgType = std::vector<TaskCfgType>;
		using MapTaskType = std::map<std::string,LstTaskCfgType>;

	public:
		bool StartAppModule();

	private:
		CAppModuleMng() = default;
		~CAppModuleMng() = default;

		void GetAppModuleTaskLst(const std::tuple<std::string,Json>& taskLst);
		void LoadAppModuleDll(const std::string& strModuleName);

	private:
		MapDllType m_mapDllShrPtr;
		MapTaskType m_mapTaskCfg;
	};
#define  SCAppModuleMng (common_template::CSingleton<CAppModuleMng>::GetInstance())
}