#include "VideoQueue.h"
#include <iostream>
#include "comm/CommFun.h"
#include "log4cxx/Loging.h"
#include "IVideoServiceModule.h"

using namespace std;
using namespace rtsp_net;
using namespace IF_VIDEO_SERVICE;

CVSQueue::CVSQueue() :
m_PicWidth(0), m_PicHeight(0)
{
	for (int i = 0; i < gTaskQueueNum; ++i)
	{
		m_VideoStreamQue[i].set_capacity(gTaskQueueCapacity);
	}
}

CVSQueue::~CVSQueue()
{
	Release();
}

//设置图片大小
int CVSQueue::SetPictureSize(int width, int height)
{
	//LOG_VS(INFO) << string_format("Set Picture Size Width: %d, Height:%d\n", width, height);
	m_PicWidth = width;
	m_PicHeight = height;
}

//释放图片资源
void CVSQueue::Release()
{	
	for (int i = 0; i < gTaskQueueNum; ++i)
	{
		boost::circular_buffer<st_cvMat>::iterator it = m_VideoStreamQue[i].begin();
		for (; it != m_VideoStreamQue[i].end(); ++it)
		{
			if (!(*it).srcMat.empty())
			{
				(*it).srcMat.release();
			}
		}
	}
}

void CVSQueue::Push(const BYTE* src)
{	
	st_cvMat sMatObj;
	sMatObj.srcMat.create(m_PicHeight, m_PicWidth, CV_8UC3);
	memcpy(sMatObj.srcMat.data, src, m_PicWidth * m_PicHeight * 3);
	std::string strCurTime = GetMillionTime();
	sMatObj.strShotTime = strCurTime;

	for (int i = 0; i < gTaskQueueNum; i++)
	{
		if (!m_VideoStreamQue[i].full())
		{
			m_VideoStreamQue[i].push_back(sMatObj);
		}
	}
}

void CVSQueue::Push(const cv::Mat& cvMat)
{
	st_cvMat sMatObj;
	sMatObj.strShotTime = GetMillionTime();
	sMatObj.srcMat = std::move(cvMat);

	for (int i = 0; i < gTaskQueueNum; i++)
	{
		if (!m_VideoStreamQue[i].full())
		{
			m_VideoStreamQue[i].push_back(sMatObj);
		}
	}
}

