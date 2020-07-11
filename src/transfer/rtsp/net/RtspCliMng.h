#pragma once
#include <string>
#include "CommFun.h"
#include "mutex/LibMutex.h"
#include "comm/NonCopyable.h"
#include "videostream/BasicUsageEnvironment.hh"
#include "videostream/H264VideoRTPSource.hh"
#include "IVideoServiceModule.h"
#include "time/TimeWatcher.h"
#include "ffmpeg/FFmpeg.h"
#include "time/TimeWatcher.h"
#include "RtspCli.h"
#include "VideoQueue.h"
#include "VideoModule.h"

namespace rtsp_net
{
    enum eCamRunState
    {
        CameraNormal = 1,
        CameraAbnormal = 2
    };

    //摄像头超时重连秒数
    const int nTimeOut = 3;

    class CRtspCliMng : common_template::NonCopyable
    {
        using CLockType = common_cmmobj::CLock;
        using VSQueShrPtr = std::shared_ptr<CVSQueue>;
        
    public:
        CRtspCliMng(const IF_VIDEO_SERVICE::TCameraBaseInfo& camBaseInfo);
        ~CRtspCliMng();

        //打开摄像头
        void OpenCamera();

        //关闭摄像头
        void CloseCamera();

        //更新摄像头信息
        void UpdateCamBaseInfo(const IF_VIDEO_SERVICE::TCameraBaseInfo& camBaseInfo);

        //拉流遍历接口
        inline void PullStream()
        {
            ((BasicTaskScheduler0 *)m_pScheduler)->SingleStep();

            //检测是否重连
            CheckReConnet();

            //检测是否上报
            CheckCameraReport();
        }

    private:
        //获取摄像头RTSP地址
        bool GetRtspUrl();

        //拉流回调函数
        void CallBackFunc(SPropRecord* pRecord, int nRecords, u_int8_t* fReceiveBuffer, unsigned frameSize);

        //获取摄像头信息
        inline std::string GetCamCode()
        {
            CLockType lock(&m_CamBaseInfoMutex);
            return std::string(m_CamBaseInfo.strCameraCode);
        }

        //获取摄像头信息
        inline std::string GetCamIP()
        {
            CLockType lock(&m_CamBaseInfoMutex);
            return std::string(m_CamBaseInfo.strIP);
        }

        //获取摄像头信息
        inline std::string GetCamUserName()
        {
            CLockType lock(&m_CamBaseInfoMutex);
            return std::string(m_CamBaseInfo.strUserName);
        }

        //获取摄像头信息
        inline std::string GetCamPwd()
        {
            CLockType lock(&m_CamBaseInfoMutex);
            return std::string(m_CamBaseInfo.strPassword);
        }


        //检测摄像机回调数据是否有PPS
        inline bool CheckHavePPS(int nRecords)
        {
            //如果没有sps和pps，解码会失败, nRecords < 2: 表示没有PPS。 
            if (nRecords < 2)
            {
                if (m_NoPPSLogTimer.IsTimeOut())
                {
                    //LOG_VS(aas_commonobj::WARN) << string_format("SPropRecord is No PPS | %s afterGettingFrame SPropRecords is: %d\n", GetCamCode() + "(" + GetCamIP() + ")", nRecords);
                    m_NoPPSLogTimer.ResetStartTime();
                }
                return false;
            }
            return true;
        }

        //设置队列图片大小
        inline bool SetQuePicSize(VSQueShrPtr& vsQueShrPtr, int nWidth, int nHeight)
        {
            //获取第一张图片时，设置队列图片大小
            if (m_FirstPicFlag)
            {
                m_FirstPicFlag = false;
                vsQueShrPtr->SetPictureSize(nWidth, nHeight);
                ReportCameraRunState(CameraNormal);
            }
        }

        //拉流按间隔写Debug日志
        inline bool WritePSLog()
        {
            if (m_PSLogTimer.IsTimeOut())
            {
                //LOG_VS(aas_commonobj::DEBUG) << string_format("PullStream To Queue: %s\n", GetCamCode() + "(" + GetCamIP() + ")");
                m_PSLogTimer.ResetStartTime();
            }
        }

        //检测是否重连
        inline void CheckReConnet()
        {
            if (IsTimeOut(m_TimeForReConn, nTimeOut))
            {
                //LOG_VS(aas_commonobj::INFO) << string_format("Camaro Reconnect: %s\n", GetCamCode() + "(" + GetCamIP() + ")");

                CloseCamera();
                OpenCamera();

                //重置是否为拉到的第一张图片。
                m_FirstPicFlag = true;

                //重置重连检测初始时间
                m_TimeForReConn = time(NULL);
            }
        }

        //检测是否上报
        inline void CheckCameraReport()
        {
            if (IsTimeOut(m_TimeForCamRept, IF_VIDEO_SERVICE::gCameraStateReportInterval))
            {
                ReportCameraRunState(CameraAbnormal);

                //重置上报检测初始时间
                m_TimeForCamRept = time(NULL);
            }
        }

        //上报摄像头状态
        inline void ReportCameraRunState(const eCamRunState& CamRunState)
        {
            //state状态值，1表示运行正常，2表示运行异常。
            std::string strCamState = "Normal";
            if (CamRunState == CameraAbnormal)
            {
                strCamState = "AbNormal";
            }

            if (SVSModule.ReportCameraRunState(GetCamCode(), (int)CamRunState))
            {
                //LOG_VS(aas_commonobj::INFO) << string_format("Report Camera %s Run State Success: %s\n", strCamState, GetCamCode() + "(" + GetCamIP() + ")");
            }
            else
            {
                //LOG_VS(aas_commonobj::ERROR) << string_format("Report Camera %s Run State Fail: %s\n", strCamState, GetCamCode() + "(" + GetCamIP() + ")");
            }
        }

    private:
        common_cmmobj::CMutex m_CamBaseInfoMutex;
        IF_VIDEO_SERVICE::TCameraBaseInfo m_CamBaseInfo;

        time_t m_TimeForCamRept;
        time_t m_TimeForReConn;

        std::shared_ptr<common_cmmobj::CFFmpeg> m_FmpegPtr;
        std::weak_ptr<CVSQueue> m_VSQueWekPtr;
        std::shared_ptr<CRtspCli> m_RtspClientPtr;

        TaskScheduler* m_pScheduler;
        UsageEnvironment* m_pEnv;
        std::string m_strRtspUrl;
        bool m_FirstPicFlag;
        bool m_IsOpenCamera;
        
        common_template::TimerSeconds m_PSLogTimer;
        common_template::TimerSeconds m_NoPPSLogTimer;
    };
}