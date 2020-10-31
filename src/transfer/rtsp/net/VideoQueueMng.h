#pragma once
#include <string>
#include <map>
#include <memory>
#include "mutex/RWLock.hpp"
#include "comm/Singleton.h"
#include "comm/NonCopyable.h"
#include "IVideoServiceModule.h"
#include "VideoQueue.h"

namespace rtsp_net
{
    using BYTE = unsigned char;
    class CVSQueueManager : common_template::CSingleton<CVSQueueManager>
    {
        friend class common_template::CSingleton<CVSQueueManager>;

        using VSQueWekPtr = std::weak_ptr<rtsp_net::CVSQueue>;

    public:
        //队列添加数据
        void Push(const std::string& strCamCode, const cv::Mat& srcMat);

        //添加队列对象
        void AddQueueObj(const std::string& strCamCode);

        //删除队列对象
        void DelQueueObj(const std::string& strCamCode);

        //获取队列对象
        void GetVSQueueObj(const std::string& strCamCode, VSQueWekPtr& vsQueWekPtr);
        
        //从行为队列获取图片
        void GetActImage(const std::string& strCamCode, IF_VIDEO_SERVICE::eActType ActType, IF_VIDEO_SERVICE::st_cvMat& srcMat);

        //从人脸队列获取图片
        void GetCmmImage(const std::string& strCamCode, IF_VIDEO_SERVICE::st_cvMat& srcMat);

        //检测队列对象是否存在
        bool QueueIsExist(const std::string& strCamCode);

    protected:
        CVSQueueManager();
        ~CVSQueueManager();

    private:
        std::map<std::string, std::shared_ptr<rtsp_net::CVSQueue> > m_mapVSObjs;
        common_cmmobj::WfirstRWLock m_rwLock;
    };

    #define SVSQueueManager (common_template::CSingleton<CVSQueueManager>::GetInstance())
}

