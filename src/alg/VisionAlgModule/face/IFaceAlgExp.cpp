#include "IFaceAlgExp.h"
#include "tuple/TpApply.h"
#include "log4cxx/Loging.h"
#include "FaceAlgMng.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;

extern "C" bool InitModuleDll(common_template::Any&& anyObj)
{
	return common_template::CSingleton<Vision_FaceAlg::CFaceAlgMng>::GetInstance().Init(std::move(anyObj));
}