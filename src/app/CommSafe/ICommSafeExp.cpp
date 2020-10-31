#include "ICommSafeExp.h"
#include "CommSafeMng.h"

extern "C" bool InitModuleDll(common_template::Any&& anyObj)
{
    return common_template::CSingleton<COMMSAFE_APP::CCommSafeMng>::GetInstance().Init(std::move(anyObj));
}