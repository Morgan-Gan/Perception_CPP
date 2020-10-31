#pragma once
#include <string>
#include <vector>
#include "comm/Singleton.h"
#include "SqliteDbSvr.h"
#include "RedisDbCli.h"

namespace MAIN_MNG
{
	class CDbMng : public common_template::CSingleton<CDbMng>
	{
		friend class common_template::CSingleton<CDbMng>;
		using RdCliType = MAIN_MNG::CRedisDbCli;
		using RdCliShrPtrType = std::shared_ptr<RdCliType>;

		using SqliteSvrType = MAIN_MNG::SqliteDbSvr;
		using SqliteSvrShrPtrType = std::shared_ptr<SqliteSvrType>;

	public:
		bool InitDb();

		bool SysCfgDBInit();
		void SysCfgDBOpt(bool bInit = true);

	private:
		CDbMng() = default;
		~CDbMng() = default;

	private:
		RdCliShrPtrType m_RdCliShrPtr = RdCliShrPtrType(new RdCliType());
		SqliteSvrShrPtrType m_SqliteSvrShrPtr = SqliteSvrShrPtrType(new SqliteSvrType());
	};

#define SCDbMng (common_template::CSingleton<CDbMng>::GetInstance())
}