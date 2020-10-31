#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <sys/time.h>
#include <sys/syscall.h>
#include "comm/CommDefine.h"
#include "rknn/rknn.h"

namespace Vision_DetectAlg
{
    class CDetectAlg
    {
        using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
        using AlgModelMapType = std::map<std::string,rknn_context>;
        using ResultReportTopicLstType = std::vector<std::string>;
        using ResultReportTopicMapType = std::map<std::string,int>;

    public:
        CDetectAlg() = default;
        ~CDetectAlg(){ Release();};

		bool Init(const MsgBusShrPtr& ptrMsgBus,const Json& taskCfg,const Json& algCfg,const Json& DataSrcCfg);

    private:
        bool AlgInit(const Json& algCfg);

        bool DataSrcInit(const Json& DataSrcCfg);

        void ProcVideoMat(const std::string& strIp,const std::string& strCameCode,const cv::Mat& matImg);

        void PreprocImg(const cv::Mat& srcImg,cv::Mat& dstImg);

        void Release();

    private:
        MsgBusShrPtr m_ptrMsgBus;

        ResultReportTopicMapType m_mapResultReportTopic;

        AlgModelMapType m_mapAlgModel;
    };
    
}