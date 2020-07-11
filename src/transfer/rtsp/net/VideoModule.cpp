#include "VideoModule.h"
#include "VideoMng.h"
#include "VideoQueueMng.h"
#include "PullStreamMng.h"
#include "IVideoServiceModule.h"

using namespace std;
using namespace rtsp_net;
using namespace IF_VIDEO_SERVICE;

extern "C" IVideoStreamModule* GetVSModuleInstance()
{
	return &SVSModule;
}

bool CVSModule::InitModule(IVideoStreamCallback * pCallbackObj)
{
	if (!m_pBDRCallBack)
	{
		m_pBDRCallBack = pCallbackObj;
	}

	return true;
}

bool CVSModule::StartModule(const IF_VIDEO_SERVICE::MapCamBaseInfo& mapCamera)
{
	SVideoMng.Start(mapCamera);
	return true;
}

CVSModule::CVSModule(): m_pBDRCallBack(NULL)
{
}

CVSModule::~CVSModule()
{
}

//从行为队列获取图片
void CVSModule::GetActImage(const std::string& strCameraCode, eActType ActType, st_cvMat& stcvMat)
{
	SVSQueueManager.GetActImage(strCameraCode, ActType, stcvMat);
}

//从人脸队列获取图片
void CVSModule::GetFaceImage(const std::string& strCameraCode, st_cvMat& stcvMat)
{
	SVSQueueManager.GetFaceImage(strCameraCode, stcvMat);
}

//同步添加摄像机数据
void CVSModule::SyncAddCameraInfo(const std::vector<TCameraBaseInfo>& vecCameraBaseInfo)
{
	SVideoMng.SyncAddCameraInfo(vecCameraBaseInfo);
}

//同步删除摄像机数据
void CVSModule::SyncDelCameraInfo(const std::vector<TCameraBaseInfo>& vecCameraBaseInfo)
{
	SVideoMng.SyncDelCameraInfo(vecCameraBaseInfo);
}

//同步修改摄像机数据
void CVSModule::SyncModifyCameraInfo(const std::vector<TCameraBaseInfo>& vecCameraBaseInfo)
{
	SVideoMng.SyncModifyCameraInfo(vecCameraBaseInfo);
}

//上报摄像机运行状态
bool CVSModule::ReportCameraRunState(const string& CameraCode, const int32_t state)
{
	if (!m_pBDRCallBack)
	{
		return false;
	}

	return m_pBDRCallBack->ReportCameraRunState(CameraCode, state);
}