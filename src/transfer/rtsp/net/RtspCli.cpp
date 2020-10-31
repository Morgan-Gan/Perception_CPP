#include "RtspCli.h"
#include <sstream>
#include "videostream/DigestAuthentication.hh"
#include "log4cxx/Loging.h"
#include "comm/CommFun.h"
#include "VideoCommFunc.h"

using namespace std;
using namespace rtsp_net;
using namespace common_template;
using namespace common_cmmobj;
using namespace IF_VIDEO_SERVICE;

//默认情况下，打印输出
#define RTSP_CLIENT_VERBOSITY_LEVEL 1

/*
接收缓冲区大小
每次回调afterGettingFrame之unsigned frameSize的大小：
老摄像机：平均为8000 * 4 约为30000(30k)
新摄像机：平均为20000 * 4 约为80000(80k)
*/
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 500000 //相当于新摄像机6张图片大小

//live555框架回调函数
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

//live555框架事件处理函数
void subsessionAfterPlaying(void* clientData);
void subsessionByeHandler(void* clientData);
void streamTimerHandler(void* clientData);

//设置每个流的子会话
void setupNextSubsession(RTSPClient* rtspClient);

//关闭流对象
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	do 
    {
		UsageEnvironment& env = rtspClient->envir();                // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) 
        {
			delete[] resultString;
			break;
		}

        // Create a media session object from this SDP description:
		char* const sdpDescription = resultString;
		scs.session = MediaSession::createNew(env, sdpDescription);
		delete[] sdpDescription; 
		if (scs.session == NULL) 
        {
			break;
		}
		else if (!scs.session->hasSubsessions()) 
        {
			break;
		}

		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		scs.iter = new MediaSubsessionIterator(*scs.session);
		setupNextSubsession(rtspClient);
		return;
	} while (0);

	// An unrecoverable error occurred with this stream.
	shutdownStream(rtspClient);
}

#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient) 
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	ourRTSPClient* pOurRTSPClient = (ourRTSPClient*)rtspClient;
	TCameraBaseInfo& CameraBaseInfo = pOurRTSPClient->m_CamBaseInfo;
	Authenticator Auth(CameraBaseInfo.strUserName.c_str(), CameraBaseInfo.strPassword.c_str());

	scs.subsession = scs.iter->next();
	if (scs.subsession != NULL) 
    {
		if (!scs.subsession->initiate()) 
        {
			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		}
		else 
        {
			if (scs.subsession->rtcpIsMuxed()) 
            {
				//env << "client port " << scs.subsession->clientPortNum();
			}
			else 
            {
				//env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
			}

			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP, False, &Auth);
		}
		return;
	}

	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	if (scs.session->absStartTime() != NULL) 
    {
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime(), 1.0f, &Auth);
	}
	else 
    {
		scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, (double)0, (double)-1, (float)0, &Auth);
	}
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	do 
    {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) 
        {
			break;
		}

		//env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
		if (scs.subsession->rtcpIsMuxed()) 
        {
			//env << "client port " << scs.subsession->clientPortNum();
		}
		else 
        {
			//env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
		}

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)
		scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());

		// perhaps use your own custom "MediaSink" subclass instead
		if (scs.subsession->sink == NULL) 
        {
			//env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
			//	<< "\" subsession: " << env.getResultMsg() << "\n";
			break;
		}

		scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
		((DummySink*)scs.subsession->sink)->m_CallBackFunc = ((ourRTSPClient*)rtspClient)->m_CallBackFunc;

		//env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession 

		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
			subsessionAfterPlaying, scs.subsession);

		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (scs.subsession->rtcpInstance() != NULL) 
        {
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
		}
	} while (0);

    SafeDeleteArray(resultString);

	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	Boolean success = False;
	do 
    {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode == 0)
        {
			break;
		}
	} while (0);
	delete[] resultString;
}

void subsessionAfterPlaying(void* clientData)
 {
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL) 
    {
		if (subsession->sink != NULL) return; // this subsession is still active
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) 
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir();

	subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) 
{
	ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
	StreamClientState& scs = rtspClient->scs; // alias
	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) 
{
	UsageEnvironment& env = rtspClient->envir();
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs;

	if (scs.session != NULL) 
    {
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) 
        {
			if (subsession->sink != NULL) 
            {
				Medium::close(subsession->sink);
				subsession->sink = NULL;

				if (subsession->rtcpInstance() != NULL) 
                {
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}

				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) 
        {
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
	}

	Medium::close(rtspClient);
}

void CRtspCli::Open(UsageEnvironment& env, char const* rtspURL, TCameraBaseInfo& m_CamBaseInfo, FrameCallBackFunc CallBackFunc)
{
	m_pRtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, m_CamBaseInfo.strCameraCode.c_str());
	if (m_pRtspClient == NULL) 
    {
		return;
	}

	m_pRtspClient->m_CallBackFunc = CallBackFunc;
	SetCameraBaseInfo(m_CamBaseInfo, m_pRtspClient->m_CamBaseInfo);

	Authenticator Auth(m_CamBaseInfo.strUserName.c_str(), m_CamBaseInfo.strPassword.c_str());
	m_pRtspClient->sendDescribeCommand(continueAfterDESCRIBE, &Auth);
}

void CRtspCli::Close()
{
	shutdownStream(m_pRtspClient);
}

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) 
{
		return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}
 
ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
	: RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1),
	m_CallBackFunc(NULL)
{
}
 
ourRTSPClient::~ourRTSPClient() 
{
}
 
StreamClientState::StreamClientState() : 
iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) 
{
}
 
StreamClientState::~StreamClientState() 
{
	delete iter;
	if (session != NULL) 
    {
		//删除会话，并取消调度任务
		UsageEnvironment& env = session->envir();
		env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
		Medium::close(session);
	}
}
 
DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) 
{
	return new DummySink(env, subsession, streamId);
}
 
DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
	: MediaSink(env),
	m_CallBackFunc(NULL),
	fSubsession(subsession) 
{
		fStreamId = strDup(streamId);
		fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}
 
DummySink::~DummySink() 
{
    SafeDeleteArray(fReceiveBuffer);
    SafeDeleteArray(fStreamId);
}
 
void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
struct timeval presentationTime, unsigned durationInMicroseconds) 
{
	DummySink* sink = (DummySink*)clientData;

	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}
 
void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
struct timeval presentationTime, unsigned /*durationInMicroseconds*/) 
{
	unsigned int Num = 0;
	unsigned int &SPropRecords = Num;
	SPropRecord* p_record = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), SPropRecords);

	if (frameSize > 20)
	{
		//回调接口
		m_CallBackFunc(p_record, SPropRecords, fReceiveBuffer, frameSize);
	}

	//继续请求下一帧
	continuePlaying();
}
 
Boolean DummySink::continuePlaying() 
{
	if (fSource == NULL) return False;
 
	//从输入源请求下一帧数据
	memset(fReceiveBuffer, 0 , DUMMY_SINK_RECEIVE_BUFFER_SIZE);
	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,afterGettingFrame, this, onSourceClosure, this);
	return True;
}