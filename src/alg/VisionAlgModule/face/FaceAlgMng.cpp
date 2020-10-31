#include "FaceAlgMng.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include <vector>

using namespace std;
using namespace Vision_FaceAlg;
using namespace common_cmmobj;
using namespace common_template;

CFaceAlgMng::CFaceAlgMng() :
m_ptrFaceAlg(nullptr),
m_ptrMsgBus(nullptr)
{

}

bool CFaceAlgMng::Init(common_template::Any&& anyObj)
{
    using TupleTypeInit = tuple<MsgBusShrPtr, Json, Json,Json>;
    return apply(bind(&CFaceAlgMng::InitParams, this, placeholders::_1, placeholders::_2, placeholders::_3,placeholders::_4), anyObj.AnyCast<TupleTypeInit>());
}

bool CFaceAlgMng::InitParams(const MsgBusShrPtr& ptrMsgBus, const Json& algCfg,const Json& transferCfg,const Json& dbCfg)
{
    // 消息总线绑定
	if(!ptrMsgBus)
	{
		return false;
	}
	m_ptrMsgBus = ptrMsgBus;

    //消息订阅
    string&& strTopic = string(algCfg[strAttriType]) + string(algCfg[strAttriIdx]);
	auto func = [this](const string& strCmd,const Json& jTaskCfg,const Json& jAlgCfg,const vector<Json>& jLstDataSrc)->bool{return CreateAlgObj(strCmd,jTaskCfg,jAlgCfg,jLstDataSrc);};
	m_ptrMsgBus->Attach(move(func),strTopic);
    return true;
}

bool CFaceAlgMng::CreateAlgObj(const string& strCmd,const Json& jTaskCfg,const Json& jAlgCfg,const std::vector<Json>& jLstDataSrc)
{
    if(strAlgObjCreate == strCmd)
	{
		//算法对象与模型一一对应，多个数据源排队等待算法对象计算
		if(!m_ptrFaceAlg)
		{
			m_ptrFaceAlg = FaceAlgShrPtr(new CFaceAlg());
		}
			
		for(auto DataSrc : jLstDataSrc)
		{
			m_ptrFaceAlg->Init(m_ptrMsgBus,jTaskCfg,jAlgCfg,DataSrc);
		}
	}

    return true;
}