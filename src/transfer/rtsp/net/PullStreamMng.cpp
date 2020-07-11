#include "PullStreamMng.h"
#include "stdio.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <bitset>
#include "log4cxx/Loging.h"
#include "CommFun.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;
using namespace IF_VIDEO_SERVICE;
using namespace rtsp_net;

//添加拉流对象
void CPullStreamMng::AddPullStreamObj(const TCameraBaseInfo& camBaseInfo)
{
	unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapPSObjs.find(camBaseInfo.strCameraCode);
	if (Pos == m_mapPSObjs.end())
	{
		m_mapPSObjs.emplace(camBaseInfo.strCameraCode, make_shared<CPullStream>(camBaseInfo));
		LOG_INFO("rtsp_net") << string_format("Add PullStream Object: %s\n", camBaseInfo.strCameraCode.c_str());
	}
	else
	{
		LOG_WARN("rtsp_net") << string_format("Add PullStream Object | Alreay Exist: %s\n", camBaseInfo.strCameraCode.c_str());
	}
}

//删除拉流对象
void CPullStreamMng::DelPullStreamObj(const string& strCamCode)
{
	unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapPSObjs.find(strCamCode);
	if (Pos != m_mapPSObjs.end())
	{
		LOG_INFO("rtsp_net") << string_format("Delete PullStream Object: %s\n", strCamCode.c_str());
		m_mapPSObjs.erase(Pos);
	}
	else
	{
		LOG_WARN("rtsp_net") << string_format("Delete PullStream Object | No Exist: %s\n", strCamCode.c_str());
	}
}

//启动拉流对象
void CPullStreamMng::StartPullStreamObj(const string& strCamCode)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapPSObjs.find(strCamCode);
	if (Pos != m_mapPSObjs.end())
	{
		if (!Pos->second->Start())
		{
			LOG_ERROR("rtsp_net") << string_format("Start PullStream Object Fail: %s\n", strCamCode.c_str());
		}
	}
}

//检测拉流对象是否存在
bool CPullStreamMng::CameraIsExist(const string& strCamCode)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapPSObjs.find(strCamCode);
	if (Pos != m_mapPSObjs.end())
	{
		return true;
	}

	return false;
}

//更新摄像头信息
void CPullStreamMng::UpdateCamBaseInfo(const TCameraBaseInfo& camBaseInfo)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapPSObjs.find(camBaseInfo.strCameraCode);
	if (Pos != m_mapPSObjs.end())
	{
		Pos->second->UpdateCameraBaseInfo(camBaseInfo);
		LOG_INFO("rtsp_net") << string_format("UpdateCamBaseInfo: %s\n", camBaseInfo.strCameraCode);
	}
	else
	{
		LOG_INFO("rtsp_net") << string_format("UpdateCamBaseInfo | No Exist: %s\n", camBaseInfo.strCameraCode);
	}
}