#pragma once
#include <string>
#include <functional>
#include "comm/CommFun.h"
#include "comm/Any.h"
namespace IF_RPC_SERVICE
{
    using Func = std::function<void()>;
    extern "C" bool GetRpc(common_template::Any&& RpcClient, common_template::Any&& RpcService);
    extern "C" bool CallRpcFunc(Func&& func, const std::string& strFuncName, common_template::Any&& AnyClientObj);
    extern "C" void RpcInit(const char* pAppIp, int nApport, int nMaxRpcConn);
}