#pragma once
#include <functional>
#include <string>
#include "msgbus/MessageBus.h"

namespace common_template
{
    class AccessServiceClient
    {
    public:
        AccessServiceClient() = default;
        ~AccessServiceClient() = default;

    public:
        void online(const std::string&,const std::string&,const std::string&){}
        void alive(const std::string&){}
    };

    using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
    using RPCServicePtr = std::shared_ptr<AccessServiceClient>;

    template<class R, class C, class ...DArgs, class ...Args>
	inline bool RpcWrap(const std::string & strFunName,
                        const std::string& strGetRpcTopic,
                        const std::string& strSendRpcTopic,
                        const MsgBusShrPtr& ptrMsgBus,
                        R(C::* f)(DArgs...), Args && ...args)
	{
		if (!ptrMsgBus)
		{
			return false;
		}
		common_template::Any rpcClient, rpcService;
		ptrMsgBus->SendReq<bool, common_template::Any&&, common_template::Any&&>(std::move(rpcClient), std::move(rpcService), strGetRpcTopic);
		if (rpcClient.IsNull() || rpcService.IsNull())
		{
			return false;
		}

		std::function<void()> Func = std::bind(f, rpcService.AnyCast<RPCServicePtr>(), std::forward<Args>(args)...);
		ptrMsgBus->SendReq<bool, std::function<void()>&&, const std::string&, common_template::Any&&>(std::move(Func), strFunName, std::move(rpcClient), strSendRpcTopic);
		return true;
	}
}