#pragma once
#include <stdio.h>
#include <stddef.h>
#include <vector>
#include "boost/circular_buffer.hpp"
#include "comm/NonCopyable.h"
#include "IVideoServiceModule.h"

namespace rtsp_net
{
    const int gTaskQueueNum = 3;      //任务队列数
    const int gTaskQueueCapacity = 2; //任务队列容量

    typedef long unsigned int size_t;
    typedef unsigned char BYTE;

    typedef enum
    {
        FaceQueue = 0,
        ActQueue = 1,
        SelfDefActQueue = 2
    }eQueueType;

    class CVSQueue: common_template::NonCopyable
    {
    public:
        CVSQueue();
        ~CVSQueue();

        //添加队列数据
        void Push(const cv::Mat& srcMat);
        void Push(const BYTE* src);

        //设置图片大小
        int SetPictureSize(int width, int height);

        //释放图片资源
        void Release();

        void Pop(eQueueType QueueType, IF_VIDEO_SERVICE::st_cvMat& srcMat)
        {
            if (m_VideoStreamQue[QueueType].empty())
            {
                return;
            }

            srcMat = m_VideoStreamQue[QueueType].front();
            m_VideoStreamQue[QueueType].pop_front();
        }

        //获取行为识别图片
        void GetActImage(IF_VIDEO_SERVICE::st_cvMat& srcMat)
        {
            Pop(ActQueue, srcMat);
        }

        //获取行为识别图片
        void GetSelfDefActImage(IF_VIDEO_SERVICE::st_cvMat& srcMat)
        {
            Pop(SelfDefActQueue, srcMat);
        }			

        //获取人脸识别图片
        void GetFaceImage(IF_VIDEO_SERVICE::st_cvMat& srcMat)
        {
            Pop(FaceQueue, srcMat);
        }

        int GetFaceQueSize()
        {
            return m_VideoStreamQue[FaceQueue].size();
        }

        int GetActQueSize()
        {
            return m_VideoStreamQue[ActQueue].size();
        }


    private:
        //图片宽度
        int m_PicWidth;

        //图片高度
        int m_PicHeight;

        boost::circular_buffer<IF_VIDEO_SERVICE::st_cvMat> m_VideoStreamQue[gTaskQueueNum];
    };
}