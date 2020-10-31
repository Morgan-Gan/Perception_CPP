#include "PullStream.h"
#include <iostream>
#include "VideoCommFunc.h"
#include "log4cxx/Loging.h"

using namespace rtsp_net;
using namespace std;
using namespace IF_VIDEO_SERVICE;
using namespace common_cmmobj;
using namespace common_template;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::minutes;

CPullStream::CPullStream(const TCameraBaseInfo& camBaseInfo) :
m_bRuning(false),
m_RtspClientMngPtr(new CRtspCliMng(camBaseInfo))
{
	SetCameraBaseInfo(camBaseInfo, m_TCameraBaseInfo);
}  

CPullStream::~CPullStream()
{
	m_PSThrUniPtr.reset();
	m_RtspClientMngPtr->CloseCamera();
	LOG_INFO("rtsp_net") << string_format("PullStream Thread Exit: %s\n", GetCameraCode() + "(" + GetCameraIP() + ")");
}

//开始拉流
bool CPullStream::Start()
{
	if (m_bRuning)
	{
		return false;
	}

	//打开摄像头
	m_RtspClientMngPtr->OpenCamera();

	//创建拉流线程
	m_PSThrUniPtr.reset(new CThread(boost::bind(&CPullStream::PullStreamLoop, this), 0, 0,"",-1));

	m_bRuning = true;

	return true;
}

//拉流线程循环体
void CPullStream::PullStreamLoop()
{
	m_RtspClientMngPtr->PullStream();	
}

//更新摄像头信息
void CPullStream::UpdateCameraBaseInfo(const TCameraBaseInfo& camBaseInfo)
{
	CLockType lock(&m_TCameraBaseInfoMutex);
	SetCameraBaseInfo(camBaseInfo, m_TCameraBaseInfo);
	m_RtspClientMngPtr->UpdateCamBaseInfo(camBaseInfo);

	LOG_INFO("rtsp_net") << string_format("UpdateCamBaseInfo: %s\n", GetCameraCode());
}