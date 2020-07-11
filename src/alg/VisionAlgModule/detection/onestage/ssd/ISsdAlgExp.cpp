#include "ISsdAlgExp.h"
#include "tuple/TpApply.h"
#include "log4cxx/Loging.h"
#include <string>

using namespace std;
using namespace common_cmmobj;
using namespace common_template;

bool InitIFParse()
{
	/*
	if (!SCCfgMng.LoadCfg())
	{
		return false;
	}

	if (!SCAlgModuleMng.LoadAlgModule())
	{
		return 0;
	}

	if (!SCAlgModuleMng.OperateAlgModule())
	{
		return 0;
	}
	*/
	return true;
}

IDETEC_SSDALG_API bool InitModuleDll(common_template::Any&& anyObj)
{
	using TupleType = std::tuple<>;
	return common_template::apply(InitIFParse, anyObj.AnyCast<TupleType>());
}