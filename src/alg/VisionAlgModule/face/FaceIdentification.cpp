#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "FaceIdentification.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace Vision_FaceAlg;

#ifndef NP_ERROR_NONE
#define NP_ERROR_NONE				  0     //正常
#endif
#ifndef NP_ERROR_LICENSE
#define	NP_ERROR_LICENSE		     -1		// 授权错误
#endif
#ifndef NP_ERROR_INIT
#define NP_ERROR_INIT				 -2     //初始化错误
#endif
#ifndef NP_ERROR_HANDLE
#define NP_ERROR_HANDLE				 -3		// 句柄错误
#endif
#ifndef NP_ERROR_MEMORY
#define NP_ERROR_MEMORY			     -4     //内存错误
#endif
#ifndef NP_ERROR_PARAM
#define NP_ERROR_PARAM				 -5     //参数错误
#endif
#ifndef NP_ERROE_CALL
#define NP_ERROE_CALL                -6     //函数调用失败
#endif
#ifdef NP_ERROE_IMAGE
#define NP_ERROE_IMAGE               -7
#endif

bool WaveID::g_bDogOpened = false;
char g_logfilename[512] = {0};

int WaveID::Initial(const char* modeldir, sem_t *pSem, int gpu_id)
{
	return 0;
}

int WaveID::SetModeCPU()
{
  return 0;
}

int WaveID::SetModeGPU(int gpu_id)
{
  	return 0;
}

int WaveID::Release()
{
	if (face_net1_ != nullptr)
	{
		//delete ((caffe::Net<float> *) face_net1_);
		face_net1_ = nullptr;
	}
	if(face_net2_ != nullptr)
	{
		//delete ((caffe::Net<float> *) face_net2_);
		face_net2_ = nullptr;
	}
	//usleep(50000);
	return 0;
}

//extract feature
int WaveID::ExtractFeature(const cv::Mat img, const cv::Rect facerect,const cv::Mat landmark, float* feature)
{
	if (gpu_id_ >= 0)
	{
		//caffe::Caffe::set_mode(caffe::Caffe::GPU);
		//caffe::Caffe::SetDevice(gpu_id_);
	}

	if (img.data == NULL || img.rows <= 0|| img.cols <= 0 || img.channels() != 3
	|| landmark.data == NULL || landmark.cols*landmark.rows != 136 || feature == NULL) 
	{
		return NP_ERROR_PARAM;
	}

	StructOutParam  OutParam;
	memset(&OutParam, 0, sizeof(StructOutParam));

	WV_FACE_ParamIn  parami;
	memset(&parami, 0, sizeof(WV_FACE_ParamIn));
	parami.nImg.height = img.rows;
	parami.nImg.width = img.cols;
	parami.nImg.nWidthStep = img.step[0];
	parami.nImg.nChannel = img.channels();
	parami.nImg.nType = WV_BGR;
	parami.nImg.pData = (unsigned char*)img.data;
	float* pData = (float*)landmark.data;
	parami.nFacePoint = new WV_FacePoints();
	parami.nFacePoint->pFaceRect.left = facerect.x;
	parami.nFacePoint->pFaceRect.top = facerect.y;
	parami.nFacePoint->pFaceRect.right = facerect.x + facerect.width -1;
	parami.nFacePoint->pFaceRect.bottom = facerect.y + facerect.height -1;
	parami.nFacePoint->nFacePointNum = 68;
	for(int i =0 ;i < 68; i++) 
	{
		parami.nFacePoint->pPoints[i].x = pData[2*i];
		parami.nFacePoint->pPoints[i].y = pData[2*i+1];
	}

	int ret = 0;
	//ret = ForwardProcess((caffe::Net<float> *)face_net1_, (caffe::Net<float> *)face_net2_, &parami, &OutParam);
	if(ret || OutParam.pdata== NULL || OutParam.pdata1 == NULL || OutParam.dim != feature_dim1_ || OutParam.dim1 != feature_dim2_
		|| OutParam.dim + OutParam.dim1 != feature_dim_) 
	{
		delete parami.nFacePoint;
		parami.nFacePoint  = nullptr;
		return NP_ERROE_CALL;
	}

	//printf("************* feature detect success\n");
	delete parami.nFacePoint;
	parami.nFacePoint  = nullptr;
	unsigned char*  features = (unsigned char*)feature;
	memcpy(features, OutParam.pdata, sizeof(float)*OutParam.dim);
	memcpy(features + sizeof(float)*OutParam.dim, OutParam.pdata1, sizeof(float)*OutParam.dim1);
	return NP_ERROR_NONE;
}

//compute the score
double WaveID::FaceVerify(const float* feature1, const float* feature2)
{
	double simi1 = 0;
	const float* pData1 = feature1;
	const float* pData2 = feature2;
	for (int i = 0; i < feature_dim1_; ++i)
	{
		simi1 += (*pData1) * (*pData2);
		pData1++;
		pData2++;
	}
	simi1 = (simi1 + 1.0) / 2.0 + 0.03;
	double simi2 = 0;
	for (int i = feature_dim1_; i < feature_dim_; ++i)
	{
		simi2 += (*pData1) * (*pData2);
		pData1++;
		pData2++;
	}
	simi2 = (simi2 + 1.0) / 2.0 ;
	double simi = 0.0;
	double thr = 0.5;
	simi = cv::min(thr*simi1 + (1-thr)*simi2+ 0.02,1.0);
	return simi;
}
