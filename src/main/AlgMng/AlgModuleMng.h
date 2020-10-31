#pragma once
#include <unordered_map>
#include "comm/Singleton.h"
#include "comm/CommDefine.h"

namespace MAIN_MNG
{
	class CAlgModuleMng : public common_template::CSingleton<CAlgModuleMng>
	{
		friend class common_template::CSingleton<CAlgModuleMng>;
		
	public:
		bool StartAlgModule();

	private:
		CAlgModuleMng() = default;
		~CAlgModuleMng() = default;

		void LoadAndInitAlgModule(std::tuple<Json,Json,Json>&& cfg);

		bool LoadAlgModule(const std::string& strModuleDllName);
		void InitAlgModule(const string& strModuleName,Json&& algCfg,Json&& tranferCfg,Json&& dbCfg);

	private:
		std::unordered_map<std::string, DllShrPtr> m_mapDllShrPtr;
	};
#define  SCAlgModuleMng (common_template::CSingleton<CAlgModuleMng>::GetInstance())
}