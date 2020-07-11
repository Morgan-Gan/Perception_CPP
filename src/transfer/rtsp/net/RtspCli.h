#pragma once
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include "videostream/liveMedia.hh"
#include "videostream/BasicUsageEnvironment.hh"
#include "comm/NonCopyable.h"
#include "IVideoServiceModule.h"
namespace rtsp_net
{
    typedef std::function<void(SPropRecord*, int, u_int8_t*, unsigned)>  FrameCallBackFunc;
    class ourRTSPClient;
    class CRtspCli: common_template::NonCopyable
    {
    public:
        CRtspCli() = default;
        ~CRtspCli() = default;
        
        //打开摄像头
		void Open(UsageEnvironment& env, char const* rtspURL, IF_VIDEO_SERVICE::TCameraBaseInfo& m_CamBaseInfo, FrameCallBackFunc CallBackFunc);
        
        //关闭摄像头
        void Close();
    private:
        ourRTSPClient* m_pRtspClient;
    };

    //维持流状态类
    class StreamClientState 
    {
    public:
        StreamClientState();
        virtual ~StreamClientState();

    public:
        MediaSubsessionIterator* iter;
        MediaSession* session;
        MediaSubsession* subsession;
        TaskToken streamTimerTask;
        double duration;
    };

    //重写客户端类
    class ourRTSPClient : public RTSPClient 
    {
        public:
            static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
                int verbosityLevel = 0,
                char const* applicationName = NULL,
                portNumBits tunnelOverHTTPPortNum = 0);

        protected:
            ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
                int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
            virtual ~ourRTSPClient();

        public:
            StreamClientState scs;
            FrameCallBackFunc m_CallBackFunc;
            IF_VIDEO_SERVICE::TCameraBaseInfo m_CamBaseInfo;
    };

    //定义数据接收类(接收流数据)
    class DummySink : public MediaSink 
    {
    public:
        static DummySink* createNew(UsageEnvironment& env,
            MediaSubsession& subsession,
            char const* streamId = NULL);

        FrameCallBackFunc m_CallBackFunc;

    private:
        DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
        virtual ~DummySink();

        static void afterGettingFrame(void* clientData, unsigned frameSize,
            unsigned numTruncatedBytes,
        struct timeval presentationTime,
            unsigned durationInMicroseconds);
        void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
        struct timeval presentationTime, unsigned durationInMicroseconds);

    private:
        virtual Boolean continuePlaying();

    private:
        u_int8_t* fReceiveBuffer;
        MediaSubsession& fSubsession;
        char* fStreamId;
    };
}