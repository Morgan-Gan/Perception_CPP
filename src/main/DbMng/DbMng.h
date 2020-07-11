#pragma once
#include <string>
#include <vector>
#include "comm/Singleton.h"

namespace MAIN_MNG
{
	class CDbMng : public common_template::CSingleton<CDbMng>
	{
		friend class common_template::CSingleton<CDbMng>;
	public:
		bool InitDb();
		bool SaveDataToRedis(const std::string& strKey, const std::string& strData);
		void GetDataFromRedis(const std::string& strKey, std::vector<std::string>& vecData);
		void DelDataFromRedis(const std::string& strKey);

	private:
		bool InitRedis();

	private:
		CDbMng();
		~CDbMng();
	};

#define SCDbMng (common_template::CSingleton<CDbMng>::GetInstance())
}