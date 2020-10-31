#pragma once
#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/log.h>
}

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "videostream/H264VideoRTPSource.hh"
#include "comm/CommDefine.h"
#include "comm/NonCopyable.h"
#include <opencv2/opencv.hpp>

namespace common_cmmobj
{
    //取流状态
    enum class eStartEndFlag
    {
        End = -1,      //结束
        Start = 0,     //开始
        OnTheWay = 1   //进行中
    };

    class CFFmpeg : common_template::NonCopyable
    {
        typedef std::function<bool(cv::Mat&, eStartEndFlag)>  FramCallBackFun;

    public:
        CFFmpeg();
        ~CFFmpeg();

        //打开H264解码器
        bool OpenH264Avcodec();

        //图片解码
        void DecodePicture(SPropRecord* p_record, uint8_t* frameBuffer, int frameLength, uint8_t** pPicBuffer);

        //打开视频文件
        bool OpenVideoSrc(const std::string& strSrcPath);

        //关闭视频文件
        bool CloseVideoSrc();

        //获取所选区间视频帧
        bool GetSectionFrame(int nBeginSec, int nEndSec, FramCallBackFun cbOfflineStudyFun);

        //获取所选区间视频帧
        bool GetFrame(FramCallBackFun cbOfflineStudyFun);

        //获取图片宽度
        int GetWidth();

        //获取图片高度
        int GetHeight();

    private:
        //AVframe To CVMat
        bool AVframeToCVMat(const AVFrame *frame, cv::Mat& cvMat);

        //执行取帧回调函数
        inline bool ExecCallBackFun(cv::Mat& cvMat, eStartEndFlag StartEndFlag)
        {
            if (cvMat.empty())
            {
                return false;
            }

            eStartEndFlag NewStartEndFlag = StartEndFlag;
            if (StartEndFlag == eStartEndFlag::Start && ++m_nFrameCount > 1)
            {
                NewStartEndFlag = eStartEndFlag::OnTheWay;  //进行中
            }

            if (!m_FramCallBackFun(cvMat, NewStartEndFlag))
            {
                return false;
            }
            else
            {
                m_bCBSuccess = true;
                return true;
            }
        }

        //设置取帧回调函数
        inline bool SetFrameCallBackFun(FramCallBackFun framCallBackFun)
        {
            if (framCallBackFun)
            {
                m_FramCallBackFun = framCallBackFun;
                return true;
            }
            else
            {
                return false;
            }
        }

    public:
        AVCodec* m_pCodec = nullptr;
        AVCodecContext* m_pCodecContext = nullptr;
        AVFrame* m_uFrame = av_frame_alloc();
        AVFrame* m_uFrameBGR = av_frame_alloc();
        uint8_t* m_uOBuffer = nullptr;
        AVFormatContext* m_pOFmtCtx = nullptr;
        SwsContext* m_pSwsContext = nullptr;
        AVPicture* m_pAVPicture = nullptr;
        unsigned char* out_bufferRGB = nullptr;

        bool m_bCBSuccess = true;
        int m_nFrameCount = 0;
        int m_nVideoIndex = -1;
        int m_nWidth = 0;
        int m_nHeight = 0;
        float m_fps = 0;
        FramCallBackFun m_FramCallBackFun;
    };
}

