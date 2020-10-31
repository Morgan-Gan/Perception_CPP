#include "RtspCliMng.h"
#include "VideoQueueMng.h"
#include <thread>
#include <chrono>
#include "comm/ScopeGuard.h"
#include "VideoCommFunc.h"
#include "VideoQueueMng.h"
#include "log4cxx/Loging.h"

extern "C" 
{
    #include "onvif/onvif_client.h"
}

using namespace std;
using namespace rtsp_net;
using namespace common_template;
using namespace common_cmmobj;
using namespace IF_VIDEO_SERVICE;

CRtspCliMng::CRtspCliMng(const TCameraBaseInfo& camBaseInfo)
	:m_TimeForCamRept(time(NULL)),
	m_TimeForReConn(time(NULL)),
	m_FmpegPtr(new CFFmpeg()),
	m_RtspClientPtr(new CRtspCli()),
	m_pScheduler(NULL),
	m_pEnv(NULL),
	m_FirstPicFlag(true),
	m_IsOpenCamera(false),
	m_PSLogTimer(3),
	m_NoPPSLogTimer(30)
{
	SetCameraBaseInfo(camBaseInfo, m_CamBaseInfo);

	//获取队列对象
	SVSQueueManager.GetVSQueueObj(GetCamCode(), m_VSQueWekPtr);

	//打开H264解码器
	m_FmpegPtr->OpenH264Avcodec();

	m_pScheduler = BasicTaskScheduler::createNew();
	m_pEnv = BasicUsageEnvironment::createNew(*m_pScheduler);
}

CRtspCliMng::~CRtspCliMng()
{
	CloseCamera();

	m_pEnv->reclaim();
	m_pEnv = NULL;
	delete m_pScheduler;
	m_pScheduler = NULL;
}

//获取摄像头RTSP地址
bool CRtspCliMng::GetRtspUrl()
{
	stringstream strStream;
	strStream << "rtsp://" << GetCamIP().c_str() << ":554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1";
	m_strRtspUrl = strStream.str();
	return true;
	struct IpcInformation ipc_info = {0};
	int ret = ONVIF_GetIPCInformation(GetCamIP().c_str(), GetCamUserName().c_str(), GetCamPwd().c_str(), 0, &ipc_info);
	if (ret == 0)
	{
		m_strRtspUrl = ipc_info.rtsp_uri;
		return true;
	}
	else
	{
		return false;
	}
}

//打开摄像头
void CRtspCliMng::OpenCamera()
{	
	if (!m_IsOpenCamera && GetRtspUrl())
	{
		m_RtspClientPtr->Open(*m_pEnv, m_strRtspUrl.c_str(), m_CamBaseInfo,
			std::bind(&CRtspCliMng::CallBackFunc, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4));

		m_IsOpenCamera = true;
	}
}

//关闭摄像头
void CRtspCliMng::CloseCamera()
{
	if (m_IsOpenCamera)
	{
		m_RtspClientPtr->Close();
		m_IsOpenCamera = false;
	}
}

//拉流回调函数videoRead
void CRtspCliMng::CallBackFunc(SPropRecord* pRecord, int nRecords, u_int8_t* fReceiveBuffer, unsigned frameSize)
{
	//检测摄像机回调数据是否有PPS
	if (!CheckHavePPS(nRecords))
	{
		return;
	}

	//判断是否I Frame(暂时去掉)
	/*
	if (!(fReceiveBuffer[0] == 0x65 || fReceiveBuffer[0] == 0x25))
	{
		return;
	}
	*/

	VSQueShrPtr vsQueShrPtr = m_VSQueWekPtr.lock();
	if (vsQueShrPtr)
	{
		//图片解码
		uint8_t* pPicBuffer = NULL;
		m_FmpegPtr->DecodePicture(pRecord, fReceiveBuffer, frameSize, &pPicBuffer);
		if (pPicBuffer == NULL)
		{
			return;
		}

		//获取第一张图片时，设置队列图片大小
		SetQuePicSize(vsQueShrPtr, m_FmpegPtr->GetWidth(), m_FmpegPtr->GetHeight());

		//图片添加到队列
		vsQueShrPtr->Push(pPicBuffer);

		//拉流按间隔写Debug日志
		WritePSLog();
	}
	
	//每次获取图片后，重置上报和重连检测初始时间
	m_TimeForCamRept = time(NULL);
	m_TimeForReConn = time(NULL);
}

//更新摄像头信息
void CRtspCliMng::UpdateCamBaseInfo(const TCameraBaseInfo& camBaseInfo)
{
	CLock lock(&m_CamBaseInfoMutex);
	SetCameraBaseInfo(camBaseInfo, m_CamBaseInfo);
}