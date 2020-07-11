#include "VideoMng.h"
#include "stdio.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <bitset>
#include "PullStreamMng.h"
#include "VideoQueueMng.h"
#include "VideoModule.h"
#include "log4cxx/Loging.h"
#include "CommFun.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;
using namespace IF_VIDEO_SERVICE;
using namespace rtsp_net;

//启动拉流和队列管理
bool CVideoMng::Start(const MapCamBaseInfo& mapCamera)
{
	//开始拉流对象
	StartPullStreamObj(mapCamera);

	return true;
}

//开始拉流对象
void CVideoMng::StartPullStreamObj(const MapCamBaseInfo& mapCamera)
{
	for (const auto& ele : mapCamera)
	{
		//添加拉流对象
		AddPullStreamObj(ele.second);

		//启动拉流对象
		SPullStreamManager.StartPullStreamObj(ele.second.strCameraCode);
	}
}

//添加拉流对象
void CVideoMng::AddPullStreamObj(const TCameraBaseInfo& camBaseInfo)
{
	//检测是否重复添加
	if (SVSQueueManager.QueueIsExist(camBaseInfo.strCameraCode))
	{
		return;
	}

	//如果摄像头没有打开 或者 没有分配任务，不需做处理(暂不考虑任务集的问题)
	string strTaskSetBinary;
	StringToBinary(camBaseInfo.strTaskSet, strTaskSetBinary);
	bitset<gMaxTaskNum> bitsetTask(strTaskSetBinary);

	//添加队列
	SVSQueueManager.AddQueueObj(camBaseInfo.strCameraCode);

	//添加拉流对象
	SPullStreamManager.AddPullStreamObj(camBaseInfo);
}

//删除拉流对象
void CVideoMng::DelPullStreamObj(const TCameraBaseInfo& camBaseInfo)
{
	//删除拉流对象
	SPullStreamManager.DelPullStreamObj(camBaseInfo.strCameraCode);

	//删除队列
	SVSQueueManager.DelQueueObj(camBaseInfo.strCameraCode);
}

//修改拉流对象
void CVideoMng::ModifyPullStreamObj(const TCameraBaseInfo& camBaseInfo)
{
	string strTaskSetBinary;
	StringToBinary(camBaseInfo.strTaskSet, strTaskSetBinary);
	bitset<gMaxTaskNum> bitsetTask(strTaskSetBinary);

	//如果摄像头没有打开 或者 未分配任务，则删除拉流对象。
	if (/*!camBaseInfo.OffOrOn || */bitsetTask.none())
	{
		DelPullStreamObj(camBaseInfo);
		return;
	}

	/*初始化时可能摄像头没有打开 或者 未分配任务，就没有创建队列、拉流对象。
	修改摄像头数据时，如果已经打开 并且 分配了任务，就要创建队列、拉流对象。
	*/
	//检测对象是存已存在
	if (!SPullStreamManager.CameraIsExist(camBaseInfo.strCameraCode))
	{
		//添加队列
		SVSQueueManager.AddQueueObj(camBaseInfo.strCameraCode);

		//添加拉流对象
		SPullStreamManager.AddPullStreamObj(camBaseInfo);

		//启动拉流对象
		SPullStreamManager.StartPullStreamObj(camBaseInfo.strCameraCode);
	}
	else
	{
		//如果对象已存在，更新拉流对象摄像头信息。
		SPullStreamManager.UpdateCamBaseInfo(camBaseInfo);
	}
}

//同步添加摄像机数据
void CVideoMng::SyncAddCameraInfo(const VecCamBaseInfo& vecCamBaseInfo)
{
	for (const auto& ele : vecCamBaseInfo)
	{
		//添加拉流对象
		AddPullStreamObj(ele);

		//启动拉流对象
		SPullStreamManager.StartPullStreamObj(ele.strCameraCode);
	}
}

//同步删除摄像机数据
void CVideoMng::SyncDelCameraInfo(const VecCamBaseInfo& vecCamBaseInfo)
{
	for (const auto& ele : vecCamBaseInfo)
	{
		DelPullStreamObj(ele);
	}
}

//同步修改摄像机数据
void CVideoMng::SyncModifyCameraInfo(const VecCamBaseInfo& vecCamBaseInfo)
{
	for (const auto& ele : vecCamBaseInfo)
	{
		ModifyPullStreamObj(ele);
	}
}