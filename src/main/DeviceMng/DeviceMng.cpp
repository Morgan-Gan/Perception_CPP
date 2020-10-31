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