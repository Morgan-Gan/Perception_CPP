#pragma once
#include "comm/CommDefine.h"
#include "DetectAlg.h"

namespace Vision_DetectAlg
{
	class CDetectAlgMng : public common_template::CSingleton<CDetectAlgMng>
	{
		friend class common_template::CSingleton<CDetectAlgMng>;

		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
		using DetectAlgShrPtr = std::shared_ptr<Vision_DetectAlg::CDetectAlg>;
		using DetectAlgShrPtrMap = std::map<std::string,DetectAlgShrPtr>;

	public:
		bool Init(common_template::Any&& anyObj);
		bool Start(common_template::Any&& anyObj);

	private:
		CDetectAlgMng() = default;
		~CDetectAlgMng() = default;

		bool InitParams(const MsgBusShrPtr& ptrMegBus, const Json& algCfg,const Json& transferCfg,const Json& dbCfg);
		bool CreateAlgObj(const string& strCmd,const Json& jTaskCfg,const Json& jAlgCfg,const std::vector<Json>& jLstDataSrc);

	private:
		MsgBusShrPtr m_ptrMsgBus;
		DetectAlgShrPtrMap m_mapDetectAlgShrPtr;
	};
	#define SDetectAlgMng (common_template::CSingleton<CDetectAlgMng>::GetInstance())
}