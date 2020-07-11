#pragma once
#include <map>
#include <memory>
#include <vector>
#include "comm/Singleton.h"
#include "mutex/RWLock.hpp"
#include "PullStream.h"
#include "comm/NonCopyable.h"

namespace rtsp_net
{
    class CPullStreamMng: public common_template::CSingleton<CPullStreamMng>
    {
        friend class common_template::CSingleton<CPullStreamMng>;

        typedef std::map<std::string, std::shared_ptr<rtsp_net::CPullStream> > PullStreamMapPtr;
        typedef std::map<std::string, IF_VIDEO_SERVICE::TCameraBaseInfo> MapCamBaseInfo;

    public:

        //添加拉流对象
        void AddPullStreamObj(const IF_VIDEO_SERVICE::TCameraBaseInfo& CameraBaseInfo);

        //删除拉流对象
        void DelPullStreamObj(const std::string& strCamCode);

        //启动拉流对象
        void StartPullStreamObj(const std::string& strCamCode);

        //检测拉流对象是否存在
        bool CameraIsExist(const std::string& strCamCode);

        //更新摄像头信息
        void UpdateCamBaseInfo(const IF_VIDEO_SERVICE::TCameraBaseInfo& camBaseInfo);

    protected:
        CPullStreamMng() = default;
        virtual ~CPullStreamMng() = default;

    private:
        PullStreamMapPtr m_mapPSObjs;
        common_cmmobj::WfirstRWLock m_rwLock;
    };

    #define SPullStreamManager (common_template::CSingleton<CPullStreamMng>::GetInstance())
}

