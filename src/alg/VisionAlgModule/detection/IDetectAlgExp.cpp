#include "IDetectAlgExp.h"
#include "tuple/TpApply.h"
#include "log4cxx/Loging.h"
#include "DetectAlgMng.h"
#include <string>

using namespace std;
using namespace common_cmmobj;
using namespace common_template;

extern "C" bool InitModuleDll(common_template::Any&& anyObj)
{
	return common_template::CSingleton<Vision_DetectAlg::CDetectAlgMng>::GetInstance().Init(std::move(anyObj));
}