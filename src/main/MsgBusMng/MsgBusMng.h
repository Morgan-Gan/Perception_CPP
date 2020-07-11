#pragma once
#include <memory>
#include "comm/Singleton.h"
#include "msgbus/MessageBus.h"

namespace MAIN_MNG
{
	class CMsgBusMng : public common_template::CSingleton<CMsgBusMng>
	{
		friend class common_template::CSingleton<CMsgBusMng>;
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
	public:
		MsgBusShrPtr GetMsgBus();

	private:
		CMsgBusMng();
		~CMsgBusMng();

	private:
		MsgBusShrPtr m_ptrMessageBus;
	};

#define  SCMsgBusMng (common_template::CSingleton<CMsgBusMng>::GetInstance())
}
