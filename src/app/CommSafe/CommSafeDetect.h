#pragma once
#include <queue>
#include "comm/CommDefine.h"
#include "rknn/rknn.h"
#include "mutex/RWLock.hpp"

namespace COMMSAFE_APP
{
    class CCommSafeDetect
    {
        using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
        using DataSrcMap = std::map<std::string,Json>;
        using TaskLst = std::vector<std::string>;
        using DetectQueueType = std::queue<Json>;

    public:
        CCommSafeDetect() = default;
        ~CCommSafeDetect() = default;

        void Init(const MsgBusShrPtr& ptrMegBus,const Json& jTaskCfg,const Json& jAlgCfg,const std::vector<Json>& vecDataSrc);
        void GetResult(std::vector<Json>& vecResult);

    private:
        void ProResultMsg(const std::string& strIp,const cv::Mat& img,const detection* dets,const int& total,const int& classes,const float& thresh);

    private:
        Json m_jTaskCfg;
        TaskLst m_lstTask;

        Json m_jAlgCfg;

        DataSrcMap m_mapDataSrc;

        common_cmmobj::WfirstRWLock m_rwLock;
        DetectQueueType m_qDetectResult;
    };
}