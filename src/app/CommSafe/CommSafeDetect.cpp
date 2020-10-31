#include "CommSafeDetect.h"
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

void CCommSafeDetect::Init(const MsgBusShrPtr& ptrMegBus,const Json& jTaskCfg,const Json& jAlgCfg,const vector<Json>& vecDataSrc)
{
    //任务列表
    m_jTaskCfg = jTaskCfg;

    Strtok(string(jTaskCfg[strAttriTaskLst]),"|",m_lstTask);
    if(m_lstTask.empty())
    {
        return;
    }

    //算法结果订阅
    m_jAlgCfg = jAlgCfg;

    string&& strCaclResultTopic = string(jTaskCfg[strAttriType]) + string(jTaskCfg[strAttriIdx]);
    auto caclResultCB = [this](const string& strIp,const cv::Mat& img,const detection* dets,const int& total,const int& classes,const float& thresh)->void {ProResultMsg(strIp,img,dets,total,classes,thresh);};
    ptrMegBus->Attach(move(caclResultCB),strCaclResultTopic);

    //算法创建和初始化
    for(auto DataSrc : vecDataSrc)
    {
        string&& strIpKey = DataSrc[strAttriIp];
        m_mapDataSrc.insert(pair<string,Json>(strIpKey,DataSrc));
    }

    string&& strCaclCreateTopic = string(jAlgCfg[strAttriType]) + string(jAlgCfg[strAttriIdx]);
    ptrMegBus->SendReq<bool,const string&,const Json&,const Json&,const vector<Json>&>(strAlgObjCreate,jTaskCfg,jAlgCfg,vecDataSrc,strCaclCreateTopic);
}

void CCommSafeDetect::ProResultMsg(const string& strIp,const cv::Mat& img,const detection* dets,const int& total,const int& classes,const float& thresh)
{
    for (auto task : m_lstTask)
    {
        cv::Mat srcImg;
        img.copyTo(srcImg);
        if(!DetectInference(srcImg,dets,total,classes,thresh,task))
        {
            continue;
        }

        //save result to mem
        Json jRecordData;
        jRecordData[strAttriTaskFlag] = string(m_jTaskCfg[strAttriType]);
        jRecordData[strAttriAlgIdx] = "0";
        jRecordData[strAttriAlertType] = task;
        jRecordData[strAttriAlertDate] = GetSecondTime();

        Json jDatasrc;
        Json jSrcCfg = m_mapDataSrc[strIp];
        jDatasrc[strAttriTransferId] = string(jSrcCfg[strAttriType]) + string(jSrcCfg[strAttriIdx]);
        jDatasrc[strAttriDesc] = string(jSrcCfg[strAttriDesc]);
        jRecordData[strAttriDataSrc] = jDatasrc;

        vector<unsigned char> buff;
        cv::imencode(".jpg", srcImg, buff);
        string strImg;
        strImg.resize(buff.size());
        memcpy(&strImg[0], buff.data(), buff.size());
        CBase64 base64Encode;
        jRecordData[strAttriImg] = base64Encode.Encode((char*)strImg.c_str(),strImg.length());

        unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
        m_qDetectResult.push(jRecordData);
        if(s32QueueSize < m_qDetectResult.size())
        {
            m_qDetectResult.pop();
        }

        LOG_INFO("yolo_alg") << string_format("save Ip(%s) img cls(%s)\n",strIp.c_str(),DetectLabsMap[task].data());
    }
}

void CCommSafeDetect::GetResult(std::vector<Json>& vecResult)
{
    unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
    int s32Size = m_qDetectResult.size();
    for(int i = 0;i < s32Size;i++)
    {
        vecResult.push_back(m_qDetectResult.front());
    }
}