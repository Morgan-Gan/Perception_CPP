#pragma once
#include "FaceDetection.h"
#include "FaceAlignment.h"

namespace Vision_FaceAlg
{
    class CFaceAlg
    {
        using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
        using FaceDetectShrPtr = std::shared_ptr<Vision_FaceAlg::FaceDetection>;
        using FaceAlignShrPtr = std::shared_ptr<Vision_FaceAlg::FaceAlignment>;
        using ResultReportTopicMapType = std::map<std::string,int>;

    public:
        CFaceAlg() = default;
        ~CFaceAlg() = default;

        bool Init(const MsgBusShrPtr& ptrMsgBus,const Json& taskCfg,const Json& algCfg,const Json& DataSrcCfg);
    
    private:
        template<class T>
        void AlgInit(T&& ptr,const Json& algJson)
        {
            std::forward<T>(ptr)->Init(algJson);
        }

        template<typename... Args>
        inline void AlgInitLst(const Json& algJson,Args&&... args)
        {
            std::initializer_list<int>{(AlgInit(std::forward<Args>(args),algJson), 0)...};
        }

        bool DataSrcInit(const Json& DataSrcCfg);

        void ProcVideoMat(const std::string& strIp,const std::string& strCameCode,const cv::Mat& matImg);

        bool Preprocess(const cv::Mat& srcImg,cv::Mat& dstImg,int& s32Width,int& s32Height);

    private:
        MsgBusShrPtr m_ptrMsgBus;

        FaceDetectShrPtr m_ptrFaceDetect = FaceDetectShrPtr(new FaceDetection());
        FaceAlignShrPtr m_ptrFaceAlign = FaceAlignShrPtr(new FaceAlignment());

        ResultReportTopicMapType m_mapResultReportTopic;
    };
}