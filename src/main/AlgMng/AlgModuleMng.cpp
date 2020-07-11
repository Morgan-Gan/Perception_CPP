#include "AlgModuleMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "CfgMng/CfgMng.h"
#include "boost/BoostFun.h"
#include "log4cxx/Loging.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;
using namespace MAIN_MNG;

const string strAlgModuleNode("AlgModule");

bool CAlgModuleMng::LoadAlgModule()
{
	/*
	nlohmann::json&& jsCfg = SCCfgMng.GetJsonCfg();
	for (auto it = jsCfg[strAlgModuleNode].begin(); it != jsCfg[strAlgModuleNode].end(); ++it)
	{
		string strModuleType(it.key());
		for (auto elm : it.value())
		{
			string&& strElmName = elm["name"];
			string&& strModuleName = string("./lib") + strElmName + string(".so");

			if (!InitAlgModule(strModuleType,strModuleName, std::move(elm)))
			{
				return false;
			}
		}
	}
*/
	return true;
}

bool CAlgModuleMng::OperateAlgModule()
{
	for (auto dll : m_mapDllShrPtr)
	{
		/*
		Mat&& src = cv::imread("./img/bus.jpg");
		TorchTensor output;
		MatVec vecBuf;
		FloatVec vecScale;

		string&& strTopic("ActDetectionProcMatExt");
		SCMsgBusMng.GetMsgBus()->SendReq<bool, const Mat&, TorchTensor&, MatVec&, FloatVec&>(src, output, vecBuf, vecScale, strTopic);

		if ("ObjDetectModule./libYolov3.so" == dll.first)
		{
			Mat&& src = cv::imread("./img/bus.jpg");
			TorchTensor output;
			MatVec vecBuf;
			FloatVec vecScale;

			string&& strTopic("Yolov3ProcMatExt");
			SCMsgBusMng.GetMsgBus()->SendReq<bool, const Mat&, TorchTensor&, MatVec&, FloatVec&>(src, output, vecBuf, vecScale, strTopic);

			LOG_INFO("systerm") << string_format("output size : %d %d |vecBuf size : %d |vecScale size : %d\n", output.size(0), output.size(1), vecBuf.size(), vecScale.size());
		}
		*/
	}
	return true;
}

CAlgModuleMng::CAlgModuleMng()
{
}

CAlgModuleMng::~CAlgModuleMng()
{
}

bool CAlgModuleMng::InitAlgModule(const string& strModuleType, const string& strModuleName, nlohmann::json&& cfgObj)
{
	DllShrPtr dllParser = DllShrPtr(new DllParser);
	if (dllParser->Load(strModuleName))
	{
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
		using TupleType = std::tuple<MsgBusShrPtr, Json, char*>;
		TupleType&& Tuple = std::make_tuple(std::forward<MsgBusShrPtr>(SCMsgBusMng.GetMsgBus()), cfgObj,(char*)strModuleName.c_str());

		dllParser->ExcecuteFunc<bool(Any&&)>("InitModuleDll", std::move(Tuple));

		m_mapDllShrPtr.insert(make_pair(strModuleType + strModuleName, dllParser));

		LOG_INFO("systerm") << string_format("load module successful %s\n",strModuleName.c_str());
	}
	else
	{
		LOG_INFO("systerm") << string_format("load module fail %s\n", strModuleName.c_str());
	}
	return true;
}
