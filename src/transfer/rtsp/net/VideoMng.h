#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "comm/Singleton.h"
#include "comm/NonCopyable.h"
#include "IVideoServiceModule.h"

namespace rtsp_net
{
    class CVideoMng: common_template::CSingleton<CVideoMng>
    {
        friend class common_template::CSingleton<CVideoMng>;
    public:
        //启动拉流
        bool Start(const IF_VIDEO_SERVICE::MapCamBaseInfo& mapCamera);

        //同步添加摄像机数据
        void SyncAddCameraInfo(const IF_VIDEO_SERVICE::VecCamBaseInfo& vecCamBaseInfo);

        //同步删除摄像机数据
        void SyncDelCameraInfo(const IF_VIDEO_SERVICE::VecCamBaseInfo& vecCamBaseInfo);

        //同步修改摄像机数据
        void SyncModifyCameraInfo(const IF_VIDEO_SERVICE::VecCamBaseInfo& vecCamBaseInfo);

    private:
        //开始任务
        void StartPullStreamObj(const IF_VIDEO_SERVICE::MapCamBaseInfo& mapCamera);

        //添加拉流对象
        void AddPullStreamObj(const IF_VIDEO_SERVICE::TCameraBaseInfo& camBaseInfo);

        //删除拉流对象
        void DelPullStreamObj(const IF_VIDEO_SERVICE::TCameraBaseInfo& camBaseInfo);

        //修改拉流对象
        void ModifyPullStreamObj(const IF_VIDEO_SERVICE::TCameraBaseInfo& camBaseInfo);

    protected:
        CVideoMng() = default;
        virtual ~CVideoMng() = default;

    };

    #define SVideoMng (common_template::CSingleton<CVideoMng>::GetInstance())   
}