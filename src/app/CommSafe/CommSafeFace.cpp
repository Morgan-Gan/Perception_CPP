#include "CommSafeFace.h"
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

const int s32QueueSize = 1;

void CCommSafeFace::Init(const MsgBusShrPtr& ptrMegBus,const Json& jTaskCfg,const Json& jAlgCfg,const vector<Json>& vecDataSrc)
{
    //任务配置保存
    m_jTaskCfg = jTaskCfg;

    ////任务列表
    Strtok(string(jTaskCfg[strAttriTaskLst]),"|",m_lstTask);
    if(m_lstTask.empty())
    {
        return;
    }
    
    ////任务数据源
    for(auto DataSrc : vecDataSrc)
    {
        string&& strIpKey = DataSrc[strAttriIp];
        m_mapDataSrc.insert(pair<string,Json>(strIpKey,DataSrc));
    }

    //算法配置保存
    m_jAlgCfg = jAlgCfg;
    
    //订阅算法结果主题
    string&& strCaclResultTopic = string(jTaskCfg[strAttriType]) + string(jTaskCfg[strAttriIdx]);
    auto caclResultCB = [this](const string& strIp,const cv::Mat& img,const detection* dets,const int& total,const int& classes,const float& thresh)->void {ProResultMsg(strIp,img,dets,total,classes,thresh);};
    ptrMegBus->Attach(move(caclResultCB),strCaclResultTopic);

    //发送算法创建主题
    string&& strCaclCreateTopic = string(jAlgCfg[strAttriType]) + string(jAlgCfg[strAttriIdx]);
    ptrMegBus->SendReq<bool,const string&,const Json&,const Json&,const vector<Json>&>(strAlgObjCreate,jTaskCfg,jAlgCfg,vecDataSrc,strCaclCreateTopic);
}

void CCommSafeFace::ProResultMsg(const string& strIp,const cv::Mat& img,const detection* dets,const int& total,const int& classes,const float& thresh)
{
  
}

void CCommSafeFace::GetResult(std::vector<Json>& vecResult)
{
    unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
    int s32Size = m_qFaceResult.size();
    for(int i = 0;i < s32Size;i++)
    {
        vecResult.push_back(m_qFaceResult.front());
    }
}