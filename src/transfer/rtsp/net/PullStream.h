#pragma once
#include <string>
#include "mutex/Condition.h"
#include "mutex/LibMutex.h"
#include "thread/Thread.h"
#include "comm/NonCopyable.h"
#include "RtspCliMng.h"
#include "comm/CommFun.h"
#include "IVideoServiceModule.h"
namespace rtsp_net
{
    class CPullStream : common_template::NonCopyable
    {
        using CLockType = common_cmmobj::CLock;
        using CMutexType = common_cmmobj::CMutex;
        using CThreadType = common_cmmobj::CThread;

    public:
        explicit CPullStream(const IF_VIDEO_SERVICE::TCameraBaseInfo& tCameraBaseInfo);
        ~CPullStream();

        //启动拉流
        bool Start();

        //拉流线程循环
        void PullStreamLoop();

        //更新摄像头信息
        void UpdateCameraBaseInfo(const IF_VIDEO_SERVICE::TCameraBaseInfo& tCameraBaseInfo);
    
    private:
        //获取摄像头信息
        inline std::string GetCameraCode()
        {
            CLockType lock(&m_TCameraBaseInfoMutex);
			return std::string(m_TCameraBaseInfo.strCameraCode);
        }

        //获取摄像头信息
        inline std::string GetCameraIP()
        {
            CLockType lock(&m_TCameraBaseInfoMutex);
            return std::string(m_TCameraBaseInfo.strIP);
        }

        //获取摄像头开始时间
        inline std::string GetCamStartTime()
        {
            CLockType lock(&m_TCameraBaseInfoMutex);
            return std::string(m_TCameraBaseInfo.strStartTime);
        }

        //获取摄像头结束时间
        inline std::string GetCamEndTime()
        {
            CLockType lock(&m_TCameraBaseInfoMutex);
            return std::string(m_TCameraBaseInfo.strEndTime);
        }

        //检测识别时段
        inline bool CheckDiscernDuration()
        {
            std::string&& strNowTime = GetCurClock();
            std::string&& strStartTime = GetCamStartTime();
            std::string&& strEndTime = GetCamEndTime();
            return (strNowTime >= strStartTime && strNowTime <= strEndTime) ? true : false;
        }

    private:
        std::string m_strRtsp;
        bool m_bRuning;

        CMutexType m_TCameraBaseInfoMutex;
        IF_VIDEO_SERVICE::TCameraBaseInfo m_TCameraBaseInfo;

        time_t m_LastAcceptTime;
		time_t m_WrongTime;

        std::shared_ptr<CRtspCliMng> m_RtspClientMngPtr;
        std::unique_ptr<CThreadType> m_PSThrUniPtr;
    };
} // namespace rtsp_net
