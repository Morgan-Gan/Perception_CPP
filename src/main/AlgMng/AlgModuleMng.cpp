#include "AlgModuleMng.h"
#include "MsgBusMng/MsgBusMng.h"
#include "CfgMng/CfgMng.h"
#include "boost/BoostFun.h"
#include "log4cxx/Loging.h"
#include "comm/FuncInitLst.h"
#include "SubTopic.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;
using namespace MAIN_MNG;

const string strAlgModuleNode("AlgModule");

bool CAlgModuleMng::StartAlgModule()
{
	//算法配置
	Json algCfg;
	if(!SCCfgMng.GetSysCfgJsonObj(strAlgNode,algCfg))
	{
		return false;
	}

	//获取传输配置
	Json tranferCfg;
	if(!SCCfgMng.GetSysCfgJsonObj(strTransferNode,tranferCfg))
	{
		return false;
	}

	//获取数据库配置
	Json dbCfg;
	if(!SCCfgMng.GetSysCfgJsonObj(strDbNode,dbCfg))
	{
		return false;
	}

	//算法模块加载和初始化
	using TupleType = tuple<Json,Json,Json>;
	TupleType&& tupleCfg = make_tuple<Json,Json,Json>(move(algCfg),move(tranferCfg),move(dbCfg));
	FuncInitLst(std::bind(&CAlgModuleMng::LoadAndInitAlgModule,this,placeholders::_1),std::move(tupleCfg));
	
	return true;
}

void CAlgModuleMng::LoadAndInitAlgModule(tuple<Json,Json,Json>&& cfg)
{
	Json algCfg,tranferCfg,dbCfg;
	tie(algCfg,tranferCfg,dbCfg) = cfg;

	for (auto ite = algCfg.begin();ite != algCfg.end();ite++)
	{
		string&& strSubNode = string(ite.key());
		for(auto elm : algCfg[strSubNode])
		{
			string&& strEnable = elm[strAttriEnable];
			if("0" == strEnable)
			{
				continue;
			}

			//加载算法模块
			string&& strDllName = string(elm[strAttriName]) + string(".so") + string(elm[strAttriVersion]);
			if(!LoadAlgModule(strDllName))
			{
				continue;
			}

			//初始化算法模块
			InitAlgModule(strDllName,move(elm),move(tranferCfg),move(dbCfg));
		}
	}
}

bool CAlgModuleMng::LoadAlgModule(const std::string& strModuleDllName)
{
	bool bRet = false;
	DllShrPtr dllParser = DllShrPtr(new DllParser);
	if (bRet = dllParser->Load(string("./lib/lib") + strModuleDllName))
	{
		m_mapDllShrPtr.insert(make_pair(strModuleDllName, dllParser));
		LOG_INFO("systerm") << string_format("load module successful %s\n",strModuleDllName.c_str());
		return true;
	}
	else
	{
		LOG_INFO("systerm") << string_format("load module fail %s\n", strModuleDllName.c_str());
	}
	return false;
}

void CAlgModuleMng::InitAlgModule(const string& strModuleName,Json&& algCfg,Json&& tranferCfg,Json&& dbCfg)
{
	auto iter = m_mapDllShrPtr.find(strModuleName);
	if (m_mapDllShrPtr.end() != iter)
	{
		using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
		using TupleType = std::tuple<MsgBusShrPtr,Json,Json,Json>;

		TupleType&& Tuple = std::make_tuple(std::forward<MsgBusShrPtr>(SCMsgBusMng.GetMsgBus()), algCfg,tranferCfg,dbCfg);

		bool bRet = false;
		bRet = iter->second->ExcecuteFunc<bool(Any&&)>("InitModuleDll", std::move(Tuple));
		if(bRet)
		{
			LOG_INFO("systerm") << string_format("init algmodule %s successful!\n", strModuleName.c_str());
		}
		else
		{
			m_mapDllShrPtr.erase(iter);
			LOG_INFO("systerm") << string_format("init algmodule %s fail!\n", strModuleName.c_str());
		}
	}
}