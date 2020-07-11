#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "VideoQueue.h"
#include "VideoQueueMng.h"
#include "log4cxx/Loging.h"
#include "CommFun.h"
#include "IVideoServiceModule.h"
#include "boost/make_shared.hpp"

using namespace std;  
using namespace rtsp_net;
using namespace common_cmmobj;
using namespace common_template;
using namespace IF_VIDEO_SERVICE;

CVSQueueManager::CVSQueueManager()
{
}

CVSQueueManager::~CVSQueueManager()
{
}

//检测队列对象是否存在
bool CVSQueueManager::QueueIsExist(const string& strCamCode)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapVSObjs.find(strCamCode);
	if (Pos != m_mapVSObjs.end())
	{
		return true;
	}
	return false;
}

//添加队列对象
void CVSQueueManager::AddQueueObj(const std::string& strCamCode)
{
	unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapVSObjs.find(strCamCode);
	if (Pos == m_mapVSObjs.end())
	{
		m_mapVSObjs.emplace(strCamCode, std::make_shared<CVSQueue>());
		//LOG_VS(INFO) << string_format("Add Queue Object: %s\n", strCamCode.c_str());
	}
	else
	{
		//LOG_VS(INFO) << string_format("Add Queue Object | Already existed: %s\n", strCamCode.c_str());
	}
}

//删除队列对象
void CVSQueueManager::DelQueueObj(const std::string& strCamCode)
{
	unique_writeguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapVSObjs.find(strCamCode);
	if (Pos != m_mapVSObjs.end())
	{
		m_mapVSObjs.erase(Pos);
		//LOG_VS(INFO) << string_format("Del Queue Object: %s\n", strCamCode.c_str());
	}
	else
	{
		//LOG_VS(INFO) << string_format("Del Queue Object | No Exist: %s\n", strCamCode.c_str());
	}
}

//获取队列对象
void CVSQueueManager::GetVSQueueObj(const std::string& strCamCode, VSQueWekPtr& vsQueWekPtr)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapVSObjs.find(strCamCode);
	if (Pos != m_mapVSObjs.end())
	{
		vsQueWekPtr = Pos->second;
	}
	else
	{
		//LOG_VS(INFO) << string_format("Get VideoStream Queue Object | No Exist: %s\n", strCamCode.c_str());
	}
}

//队列添加数据
void CVSQueueManager::Push(const std::string& strCamCode, const cv::Mat& srcMat)
{

}

//从行为队列获取图片
void CVSQueueManager::GetActImage(const std::string& strCamCode, eActType ActType, st_cvMat& srcMat)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapVSObjs.find(strCamCode);
	if (Pos != m_mapVSObjs.end())
	{
		if (ActType == eActType::SelfDef)
		{
			Pos->second->GetSelfDefActImage(srcMat);
		} 
		else
		{
			Pos->second->GetActImage(srcMat);
		}
	}
}

//从人脸队列获取图片
void CVSQueueManager::GetFaceImage(const std::string& strCamCode, st_cvMat& srcMat)
{
	unique_readguard<WfirstRWLock> rwlock(m_rwLock);
	const auto Pos = m_mapVSObjs.find(strCamCode);
	if (Pos != m_mapVSObjs.end())
	{
		Pos->second->GetFaceImage(srcMat);
	}
}

