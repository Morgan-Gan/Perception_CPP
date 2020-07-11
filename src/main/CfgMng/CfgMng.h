#pragma once
#include <map>
#include "comm/Singleton.h"
#include "CommDefine.h"
#include "CfgObj.h"
namespace MAIN_MNG
{	
	using CfgObjMap = std::map<std::string,CfgObj>;
	class CCfgMng : public common_template::CSingleton<CCfgMng>
	{
		friend class common_template::CSingleton<CCfgMng>;
	public:
		bool LoadCfg();

		void GetCfgJsonObj(const std::string& strKey,std::vector<Json>& lstCfg);

	private:
		CCfgMng() = default;
		~CCfgMng() = default;

	private:
		bool LoadJsonCfg(const std::string& strPath);
		void ReadJsonFile(const std::string& strPath, std::string& strJson);
		bool ParseJsonCfg();
		void ParseJsonNode(const std::string& strNode,const std::string& strSubNode);

	private:
		Json m_jsObj;
		CfgObjMap m_mapCfgObj;
	};

#define SCCfgMng (common_template::CSingleton<CCfgMng>::GetInstance())
}