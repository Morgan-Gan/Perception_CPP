#pragma once
#include "IVideoServiceModule.h"
#include "comm/Singleton.h"

namespace rtsp_net
{
    class CVSModule : public 
    IF_VIDEO_SERVICE::IVideoStreamModule,
    common_template::CSingleton<CVSModule>
    {
        friend class common_template::CSingleton<CVSModule>;

    public:
        virtual bool InitModule(IF_VIDEO_SERVICE::IVideoStreamCallback* pCallbackObj);
        virtual bool StartModule(const IF_VIDEO_SERVICE::MapCamBaseInfo& mapCamera);

        //从行为队列获取图片
        virtual void GetActImage(const std::string& strCameraCode, IF_VIDEO_SERVICE::eActType ActType, IF_VIDEO_SERVICE::st_cvMat& stcvMat);

        //从人脸队列获取图片
        virtual void GetFaceImage(const std::string& strCameraCode, IF_VIDEO_SERVICE::st_cvMat& stcvMat);

        //上报摄像机运行状态
        bool ReportCameraRunState(const std::string& CameraCode, const int32_t state);

        //同步添加摄像机数据
        void SyncAddCameraInfo(const std::vector<IF_VIDEO_SERVICE::TCameraBaseInfo>& vecCameraBaseInfo);

        //同步删除摄像机数据
        void SyncDelCameraInfo(const std::vector<IF_VIDEO_SERVICE::TCameraBaseInfo>& vecCameraBaseInfo);

        //同步修改摄像机数据
        void SyncModifyCameraInfo(const std::vector<IF_VIDEO_SERVICE::TCameraBaseInfo>& vecCameraBaseInfo);

    protected:
        CVSModule();
        virtual ~CVSModule();

    private:
        IF_VIDEO_SERVICE::IVideoStreamCallback* m_pBDRCallBack;
    };

    #define SVSModule (common_template::CSingleton<CVSModule>::GetInstance())
}