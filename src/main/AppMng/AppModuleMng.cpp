#include "AppModuleMng.h"
#include "SubTopic.h"
#include "CfgMng/CfgMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "comm/CommDefine.h"
#include "boost/BoostFun.h"
#include "log4cxx/Loging.h"
#include "AlgMng/AlgModuleMng.h"
#include "comm/FuncInitLst.h"
#include "CommData.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;
using namespace MAIN_MNG;

bool CAppModuleMng::StartAppModule()
{
	//获取应用配置
	Json AppCfg;
	if(!SCCfgMng.GetSysCfgJsonObj(strAppNode,AppCfg))
	{
		return false;
	}
	LOG_INFO("systerm") << AppCfg.dump();

	//配置工具的tcpsvr通信配置
	Json SouthTcpSvrCfg;
	if(!SCCfgMng.GetSysCfgJsonObj(strTransferNode + strTransferSouthTcpSvrNode,SouthTcpSvrCfg))
	{
		return false;
	}

	//应用库加载和初始化
	for (auto it = AppCfg.begin(); it != AppCfg.end(); ++it)
	{
		//加载应用动态库
		string&& strAppName = string(it.key());
		FuncInitLst(std::bind(&CAppModuleMng::LoadAppModuleDll,this,placeholders::_1),strAppName);

		//获取应用模块配置的任务列表
		tuple<string,Json>&& tupleTasks = make_tuple(strAppName,it.value());
		FuncInitLst(std::bind(&CAppModuleMng::GetAppModuleTaskLst,this,placeholders::_1),tupleTasks);

		//应用模块初始化
		using TupleType = tuple<MsgBusShrPtr,Json,LstTaskCfgType>;
		TupleType&& taskTuple = make_tuple(forward<MsgBusShrPtr>(SCMsgBusMng.GetMsgBus()),SouthTcpSvrCfg,m_mapTaskCfg[strAppName]);
		m_mapDllShrPtr[strAppName]->ExcecuteFunc<bool(Any&&)>("InitModuleDll", move(taskTuple));
	}
	
	return true;
}

void CAppModuleMng::LoadAppModuleDll(const string& strModuleName)
{
	//加载应用动态库
	auto iter = m_mapDllShrPtr.find(strModuleName);
	if(iter == m_mapDllShrPtr.end())
	{
		DllShrPtr dllPtr = DllShrPtr(new DllParser);
		if(dllPtr->Load(string("./lib/lib")+ strModuleName + string(".so")))
		{
			m_mapDllShrPtr.insert(make_pair(strModuleName, dllPtr));
			LOG_INFO("systerm") << string_format("load module successful %s\n",strModuleName.c_str());
		}
		else
		{
			LOG_INFO("systerm") << string_format("load module fail %s\n", strModuleName.c_str());
			return;
		}
	}
}

void CAppModuleMng::GetAppModuleTaskLst(const tuple<string,Json>& taskCfg)
{
	string strAppDllName;
	Json jTaskLst;
	tie(strAppDllName,jTaskLst) = taskCfg;
	
	for(auto jTask : jTaskLst)
	{
		//通过开始时间和结束时间来判断使能
		LOG_INFO("systerm") << jTask.dump();
		string&& strStartTime = jTask[strAttriStartTime];
		string&& strEndTime = jTask[strAttriEndTime];
		if(strStartTime == strEndTime)
		{
			continue;
		}

		//算法配置
		Json algCfg;
		if(!SCCfgMng.GetSysCfgJsonObj(AlgCfgMap[string(jTask[strAttriAlgIdx])],algCfg))
		{
			return;
		}

		//数据源配置
		vector<Json> vecDataSrcCfg;
		for (auto elm : jTask[strAttriDataSrc])
		{
			SCCfgMng.GetSysCfgJsonObj(string(elm[strAttriTransferId]),vecDataSrcCfg);
		}

		//插入任务列表
		m_mapTaskCfg[strAppDllName].emplace_back(make_tuple(move(jTask),move(algCfg),move(vecDataSrcCfg)));
	}
}