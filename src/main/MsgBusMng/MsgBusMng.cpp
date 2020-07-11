#include "MsgBusMng.h"

using namespace MAIN_MNG;
using namespace common_template;

CMsgBusMng::CMsgBusMng()
{
	m_ptrMessageBus.reset(new MessageBus);
}

CMsgBusMng::~CMsgBusMng()
{
}

CMsgBusMng::MsgBusShrPtr CMsgBusMng::GetMsgBus()
{
	return m_ptrMessageBus;
}
