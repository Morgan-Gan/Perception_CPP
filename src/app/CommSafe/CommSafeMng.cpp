#include "CommSafeMng.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include "endecode/Base64.h"
#include "comm/CommFun.h"
#include "CommData.h"

using namespace std;
using namespace COMMSAFE_APP;
using namespace common_cmmobj;
using namespace common_template;

bool CCommSafeMng::Init(Any&& anyObj)
{
	using TupleType = tuple<MsgBusShrPtr,Json,LstTaskCfgType>;
	return apply(bind(&CCommSafeMng::InitParams, this, placeholders::_1, placeholders::_2, placeholders::_3), anyObj.AnyCast<TupleType>());
}

bool CCommSafeMng::InitParams(const MsgBusShrPtr& ptrMegBus,const Json& southCfg,const LstTaskCfgType& lstTaskCfg)
{
	//消息总线绑定
	if(!ptrMegBus)
	{
		return false;
	}
	m_ptrMsgBus = ptrMegBus;

	//tcpserver消息订阅
	string&& strTopic = TcpSrvRecMsgTopic(string(southCfg[strAttriPort]));
	auto func = [this](const std::string& strTcpSvrPort, const std::string& strCliConn,const unsigned int& u32Cmd,const char*& pData, const int& s32DataLen)->void {RecTcpProtocolMsg(strTcpSvrPort, strCliConn,u32Cmd,pData,s32DataLen);};
	m_ptrMsgBus->Attach(move(func), strTopic);
	
	//算法创建和初始化
	for (auto val : lstTaskCfg)
	{
		//解参数
		Json taskCfg,algCfg;
		vector<Json> vecDataSrc;
		tie(taskCfg,algCfg,vecDataSrc) = val;
	
		string&& strAlgIdx = string(algCfg[strAttriType]) + string(algCfg[strAttriIdx]);
		if(0 == AlgCfgMap["0"].compare(strAlgIdx))
		{
			//目标检测
			DetectShrPtr&& shrPtr = DetectShrPtr(new CCommSafeDetect());
			m_mapDetectShrPtr.insert(pair<string,DetectShrPtr>(strAlgIdx,shrPtr));
			shrPtr->Init(ptrMegBus,taskCfg,algCfg,vecDataSrc);
		}
		else if(0 == AlgCfgMap["1"].compare(strAlgIdx))
		{
			//动作识别
		}
		else
		{
			//人脸检测
			FaceShrPtr&& shrPtr = FaceShrPtr(new CCommSafeFace());
			m_mapFaceShrPtr.insert(pair<string,FaceShrPtr>(strAlgIdx,shrPtr));
			shrPtr->Init(ptrMegBus,taskCfg,algCfg,vecDataSrc);
		}
		
	}
	return true;
}

void CCommSafeMng::RecTcpProtocolMsg(const string& strTcpSvrPort, const string& strCliConn,const unsigned int& u32Cmd,const char*& pData, const int& s32DataLen)
{
	if(!pData)
	{
		return;
	}

	string strJson(pData,s32DataLen);

	bool bRsp = false;
	Json jRspData;

	switch (u32Cmd)
	{
	case 0x41:
	{
		vector<Json> vecResult;
		for(auto ptr : m_mapDetectShrPtr)
		{
			ptr.second->GetResult(vecResult);
		}
		int s32ResultSize = vecResult.size();

		jRspData[strAttriStartIdx] = to_string(0);
		jRspData[strAttriEndIdx] = to_string(0 == s32ResultSize ? s32ResultSize : s32ResultSize - 1);
		jRspData[strAttriTotalSize] = to_string(s32ResultSize);
		bRsp = true;
	}
	break;
	case 0x42:
	{
		vector<Json> vecResult;
		for(auto ptr : m_mapDetectShrPtr)
		{
			ptr.second->GetResult(vecResult);
		}

		int s32ResultSize = vecResult.size();

		Json jReqData = Json::parse(strJson);
		jRspData[strAttriStartIdx] = string(jReqData[strAttriStartIdx]);
		jRspData[strAttriEndIdx] = to_string(0 == s32ResultSize ? s32ResultSize : s32ResultSize - 1);
		jRspData[strAttriBatchSize] = to_string(s32ResultSize);
		
		int i = 0;
		for(auto var : vecResult)
		{
			var[strAttriIdx] = to_string(i);
	
			jRspData[strAttriRecord].push_back(var);
			i++;
		}

		bRsp = true;
	}
	break;
	default:
		break;
	}

	if(!bRsp)
	{
		return;
	}

	if(m_ptrMsgBus)
	{
		LOG_INFO("systerm") << string_format("req cmd(0x%02x) and json:%s\n",u32Cmd,strJson.c_str());
		string&& strRspData = jRspData.dump();
		LOG_INFO("systerm") << string_format("rsq cmd(0x%02x) and json:%s\n",u32Cmd,strRspData.c_str());
		m_ptrMsgBus->SendReq<bool,const int &,const string&, const int&,const string&,const int&>(atoi(strTcpSvrPort.c_str()),strCliConn,u32Cmd,jRspData.dump(),1,TcpSrvSendMsgTopic(strTcpSvrPort));
	}
}