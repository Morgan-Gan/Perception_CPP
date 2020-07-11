#include "DeviceMng.h"
#include "comm/DllParser.h"
#include "msgbus/MessageBus.h"
#include "CfgMng/CfgMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "TransferMng/TransferMng.h"
#include "log4cxx/Loging.h"
#include <tuple>
#include <unordered_map>
#include <memory>
#include <functional>

using namespace std;
using namespace MAIN_MNG;
using namespace common_template;
using namespace common_cmmobj;

const string strSouthDeviceKey("AccessDevice");
const string strAccessKey("access");
const string strReportKey("report");
const string strSaveDbKey("savedb");
const string strCfgToolKey("cfgtool");

bool CDeviceMng::LoadAllDevices()
{
	//获取配置对象
	/*
	nlohmann::json&& jsCfg = SCCfgMng.GetJsonCfg();
	for (auto it = jsCfg[strSouthDeviceKey].begin(); it != jsCfg[strSouthDeviceKey].end(); ++it)
	{
		for (auto elm : it.value())
		{
			string&& strName = elm["name"];
			string&& strDeviceSo = string("./lib") + strName + string(".so");
			if (!InitDevice(strDeviceSo, std::move(elm)))
			{
				return false;
			}
		}
	}
	*/
	return true;
}

CDeviceMng::CDeviceMng()
{
}

CDeviceMng::~CDeviceMng()
{
}

bool CDeviceMng::InitDevice(const string& strSoName, nlohmann::json&& cfgObj)
{
	DllParser dllParser;
	if (dllParser.Load(strSoName))
	{
		/*
		//动态库初始化
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
		using TupleType = std::tuple<MsgBusShrPtr, nlohmann::json, string, string>;

		TupleType&& Tuple = std::make_tuple(std::forward<MsgBusShrPtr>(SCMsgBusMng.GetMsgBus()), cfgObj, std::move(SCTransferMng.GetId()), std::move(SCTransferMng.GetIp()));

		Any anyObj = Tuple;
		bool bRet = false;
		bRet = dllParser.ExcecuteFunc<bool(Any&)>("InitModuleDll", anyObj);

		//添加南向、配置工具通信配置
		if (bRet)
		{
			m_vecSouthTcpTransCfg.emplace_back(cfgObj[strAccessKey]);
			m_vecReportTransCfg.emplace_back(cfgObj[strReportKey]);
			m_vecSaveDbCfg.emplace_back(cfgObj[strSaveDbKey]);
			m_vecCfgToolCfg.emplace_back(cfgObj[strCfgToolKey]);
		}

		LOG_DEBUG("systerm") << string_format("%s init module %s\n", strSoName.c_str(), bRet ? "successfull" : "fail");
		*/
	}
	else
	{
		LOG_DEBUG("gw_warn") << string_format("%s load fail\n", strSoName.c_str());
		return false;
	}

	return true;
}

std::vector<nlohmann::json> CDeviceMng::GetSouthTcpTransCfg()
{
	return m_vecSouthTcpTransCfg;
}

std::vector<nlohmann::json> CDeviceMng::GetReportTransCfg()
{
	return m_vecReportTransCfg;
}

std::vector<nlohmann::json> CDeviceMng::GetSaveDbCfg()
{
	return m_vecSaveDbCfg;
}

std::vector<nlohmann::json> CDeviceMng::GetCfgToolCfg()
{
	return m_vecCfgToolCfg;
}